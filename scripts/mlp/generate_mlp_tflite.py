import sys
from argparse import ArgumentParser
import numpy as np
import tensorflow as tf
from tensorflow.keras import Sequential
from tensorflow.keras.layers import Input, Dense
from tensorflow.keras.datasets import mnist
from tensorflow.keras.utils import to_categorical


def main():
    parser = ArgumentParser()
    parser.add_argument('-o', '--output', type=str, required=True)
    args = parser.parse_args()

    # load and preprocess
    (x_train, y_train), (x_test, y_test) = mnist.load_data()

    # normalize pixel values (0-255) to [0, 1]
    x_train = x_train / 255.0
    x_test = x_test / 255.0

    x_train = x_train.reshape(-1, 784).astype(np.float32)  # flatten
    x_test = x_test.reshape(-1, 784).astype(np.float32)  # flatten

    # one-hot encode the labels
    y_train = to_categorical(y_train, 10)
    y_test = to_categorical(y_test, 10)

    # build the model
    model = Sequential([
        Input(shape=(784,), batch_size=1, dtype=tf.float32, name='input'),
        Dense(128, activation='relu', name='hidden'),
        Dense(10, activation='softmax', name='output')
    ])

    model.compile(
        loss='categorical_crossentropy',
        optimizer='adam',
        metrics=['accuracy']
    )

    model.fit(
        x_train,
        y_train,
        epochs=10,
        batch_size=128,
        validation_split=0.1,
        verbose=1
    )

    # evaluate on test data
    test_loss, test_acc = model.evaluate(x_test, y_test, verbose=0)
    print(f"test accuracy: {test_acc:.4f}")
    print(f"test loss: {test_loss:.4f}")

    model.summary()

    # convert to tflite

    def representative_dataset_gen():
        for idx in range(1000):
            yield [x_train[idx:idx+1].reshape(1, 784).astype(np.float32)]

    # create converter from the trained model
    converter = tf.lite.TFLiteConverter.from_keras_model(model)

    # enable optimizations to allow post-training quantization
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    # set the representative dataset for calibration
    converter.representative_dataset = representative_dataset_gen

    # force all ops to INT8
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8   # or tf.uint8
    converter.inference_output_type = tf.int8  # or tf.uint8

    converter.experimental_new_quantizer = True

    # convert the model
    tflite_quant_model = converter.convert()

    # save tflite to disk
    open(args.output, 'wb').write(tflite_quant_model)

    sys.exit(0)


if __name__ == '__main__':
    main()