import sys
from argparse import ArgumentParser
import tflite.Model
import tflite.TensorType
import math


_INPUT_QUANT_NAME = 'serving_default_input:0'
_HIDDEN_QUANT_NAME = 'sequential_1/hidden_1/MatMul;sequential_1/hidden_1/Relu;sequential_1/hidden_1/BiasAdd'
_OUTPUT_QUANT_NAME = 'sequential_1/output_1/MatMul;sequential_1/output_1/BiasAdd'
_SOFTMAX_QUANT_NAME = 'StatefulPartitionedCall_1:0'
_HIDDEN_WEIGHT_TENSOR_NAME = 'tfl.pseudo_qconst3'
_HIDDEN_BIAS_TENSOR_NAME = 'tfl.pseudo_qconst2'
_OUTPUT_WEIGHT_TENSOR_NAME = 'tfl.pseudo_qconst1'
_OUTPUT_BIAS_TENSOR_NAME = 'tfl.pseudo_qconst'

_EXPECTED_BUFFER_SIZE = {
    _INPUT_QUANT_NAME: 0,
    _HIDDEN_QUANT_NAME: 0,
    _OUTPUT_QUANT_NAME: 0,
    _SOFTMAX_QUANT_NAME: 0,
    _HIDDEN_WEIGHT_TENSOR_NAME: 100352,
    _HIDDEN_BIAS_TENSOR_NAME: 512,
    _OUTPUT_WEIGHT_TENSOR_NAME: 1280,
    _OUTPUT_BIAS_TENSOR_NAME: 40
}

_QUANTIZED_MULTIPLIER_BITWIDTH = 31


def main():
    parser = ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True)
    parser.add_argument('-o', '--output', type=str, required=True)
    parser.add_argument('-v', '--verbose', action='store_true')
    args = parser.parse_args()

    model = tflite.Model.GetRootAsModel(open(args.input, 'rb').read(), 0)

    # assume the main subgraph is at index 0.
    subgraph = model.Subgraphs(0)

    tensors = dict()
    for i in range(subgraph.TensorsLength()):
        tensor = subgraph.Tensors(i)
        tensor_type = tensor.Type()

        if tensor_type not in (tflite.TensorType.INT8, tflite.TensorType.INT32):
            raise RuntimeError(
                f'tensor index {i} has unsupported type {tensor_type}. '
                'only INT8 and INT32 tensors are allowed!'
            )

        tensor_name = tensor.Name().decode('utf-8')

        buffer_index = tensor.Buffer()
        buffer_data = model.Buffers(buffer_index).DataAsNumpy()

        assert(buffer_data is not None)

        if isinstance(buffer_data, int):
            assert(buffer_data == 0)
            buffer_data = bytearray()
        else:
            buffer_data = buffer_data.tobytes()
            
        # assume linear quantization
        quant = tensor.Quantization()
        if quant is not None:
            scales = quant.ScaleAsNumpy().tolist()
            zero_points = quant.ZeroPointAsNumpy().tolist()
        else:
            scales = [0]
            zero_points = [0]

        assert(len(buffer_data) == _EXPECTED_BUFFER_SIZE[tensor_name])

        tensors[tensor_name] = {
            'index': i,
            'scales': scales,
            'zero_points': zero_points,
            'data': buffer_data,
        }


    if args.verbose:
        for tensor_name, tensor in tensors.items():
            print(f'{tensor_name}: ')
            print(f'\tindex: {tensor["index"]}')
            print(f'\tscales: {tensor["scales"]}')
            print(f'\tzero_points: {tensor["zero_points"]}')
            print(f'\tsize: {len(tensor["data"])}')
            print('\n\n')

    def approx_eq(a, b, eps=1e-9):
        return abs(a-b) <= eps
    
    
    # inspired on tensorflow's
    # QuantizeMultiplier()
    # source: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/kernels/internal/quantization_util.cc#L53
    def quantize_multiplier(float_multiplier, max_bits=31):
        if float_multiplier <= 0.0 or float_multiplier >= 1.0:
            raise ValueError("This function only handles scale in (0, 1)")

        q, shift = math.frexp(float_multiplier)

        q_fixed = int(round(q * (1 << max_bits)))

        if q_fixed == (1 << max_bits):
            q_fixed //= 2
            shift += 1

        if shift > (max_bits - 1):
            shift = max_bits - 1
            q_fixed = (1 << max_bits) - 1

        return q_fixed, -shift
    
    def quantize_layer_multipliers(input_scale, weight_scales, output_scale, bias_scales):
        layer_multipliers = []
        layer_shifts = []
        for weight_scale, bias_scale in zip(weight_scales, bias_scales):
            layer_scale = input_scale * weight_scale 
            assert(approx_eq(layer_scale, bias_scale))
            layer_scale /= output_scale
            layer_multipler, layer_shift = quantize_multiplier(layer_scale, _QUANTIZED_MULTIPLIER_BITWIDTH)
            layer_multipliers.append(layer_multipler)
            layer_shifts.append(layer_shift)

        return layer_multipliers, layer_shifts


    def serialize_layer_data(input_zp, weight_zps, output_zp, layer_multipliers, layer_shifts):
        layer_data = bytearray()
        layer_data += input_zp.to_bytes(1, byteorder='little', signed=True)
        layer_data += output_zp.to_bytes(1, byteorder='little', signed=True)
        for weight_zp in weight_zps:
            layer_data += weight_zp.to_bytes(1, byteorder='little', signed=True)
        for layer_mutiplier in layer_multipliers:
            layer_data += layer_mutiplier.to_bytes(4, byteorder='little', signed=False)
        for layer_shift in layer_shifts:
            layer_data += layer_shift.to_bytes(4, byteorder='little', signed=True) # can be negative
        return layer_data


    assert(len(tensors[_INPUT_QUANT_NAME]['scales']) == 1)
    assert(len(tensors[_INPUT_QUANT_NAME]['zero_points']) == 1)
    assert(len(tensors[_HIDDEN_QUANT_NAME]['scales']) == 1)
    assert(len(tensors[_HIDDEN_QUANT_NAME]['zero_points']) == 1)
    assert(len(tensors[_OUTPUT_QUANT_NAME]['scales']) == 1)
    assert(len(tensors[_OUTPUT_QUANT_NAME]['zero_points']) == 1)
    assert(len(tensors[_SOFTMAX_QUANT_NAME]['scales']) == 1)
    assert(len(tensors[_SOFTMAX_QUANT_NAME]['zero_points']) == 1)
    assert(all(zp == 0 for zp in tensors[_HIDDEN_BIAS_TENSOR_NAME]['zero_points']))
    assert(all(zp == 0 for zp in tensors[_OUTPUT_BIAS_TENSOR_NAME]['zero_points']))

    layer1_multipliers, layer1_shifts = quantize_layer_multipliers(
        tensors[_INPUT_QUANT_NAME]['scales'][0],
        tensors[_HIDDEN_WEIGHT_TENSOR_NAME]['scales'],
        tensors[_HIDDEN_QUANT_NAME]['scales'][0],
        tensors[_HIDDEN_BIAS_TENSOR_NAME]['scales']
    )

    layer1_data = serialize_layer_data(
        tensors[_INPUT_QUANT_NAME]['zero_points'][0],
        tensors[_HIDDEN_WEIGHT_TENSOR_NAME]['zero_points'],
        tensors[_HIDDEN_QUANT_NAME]['zero_points'][0],
        layer1_multipliers,
        layer1_shifts
    )

    layer2_multipliers, layer2_shifts = quantize_layer_multipliers(
        tensors[_HIDDEN_QUANT_NAME]['scales'][0],
        tensors[_OUTPUT_WEIGHT_TENSOR_NAME]['scales'],
        tensors[_OUTPUT_QUANT_NAME]['scales'][0],
        tensors[_OUTPUT_BIAS_TENSOR_NAME]['scales']
    )

    layer2_data = serialize_layer_data(
        tensors[_HIDDEN_QUANT_NAME]['zero_points'][0],
        tensors[_OUTPUT_WEIGHT_TENSOR_NAME]['zero_points'],
        tensors[_OUTPUT_QUANT_NAME]['zero_points'][0],
        layer2_multipliers,
        layer2_shifts
    )

    # softmax_data = bytearray()
    # softmax_data += tensors[_SOFTMAX_QUANT_NAME]['zero_points'][0].to_bytes(1, byteorder='little', signed=True)
    # softmax_data += b'\x00\x00\x00'  # padding
    # softmax_multipler, softmax_shift = get_multiplier_and_shift(tensors[_SOFTMAX_QUANT_NAME]['scales'][0])
    # softmax_data += softmax_multipler.to_bytes(4, byteorder='little', signed=False)
    # softmax_data += softmax_shift.to_bytes(4, byteorder='little', signed=True) # can be negative

    open(args.output, 'wb').write(
        tensors[_HIDDEN_WEIGHT_TENSOR_NAME]['data'] \
        + tensors[_HIDDEN_BIAS_TENSOR_NAME]['data'] \
        + tensors[_OUTPUT_WEIGHT_TENSOR_NAME]['data'] \
        + tensors[_OUTPUT_BIAS_TENSOR_NAME]['data'] \
        + layer1_data \
        + layer2_data  # + softmax_data
    )
    
    sys.exit(0)


if __name__ == '__main__':
    main()
