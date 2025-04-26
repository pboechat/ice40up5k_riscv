import sys
from argparse import ArgumentParser
import numpy as np
import tensorflow as tf


def main():
    parser = ArgumentParser()
    parser.add_argument('-m', '--model', type=str, required=True)
    parser.add_argument('-i', '--input', type=str, required=True)
    args = parser.parse_args()

    interpreter = tf.lite.Interpreter(
        model_path=args.model,
        experimental_preserve_all_tensors=True
    )
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    input_index = input_details[0]['index']
    output_index = output_details[0]['index']

    input_data = np.frombuffer(open(args.input, 'rb').read(), dtype=np.int8)
    input_data = input_data.reshape((1, input_data.shape[0]))
    interpreter.set_tensor(input_index, input_data)

    interpreter.invoke()

    output_data = interpreter.get_tensor(output_index)

    print(interpreter.get_tensor(5).tolist()[0])
    print(interpreter.get_tensor(6).tolist()[0])

    print('******************************')
    for i, clazz in enumerate(output_data[0]):
        print(f'Class {i} => {clazz}')
    print('******************************')

    sys.exit(0)


if __name__ == "__main__":
    main()
