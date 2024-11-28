import os
import wfdb
import numpy as np
from sklearn.model_selection import train_test_split
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from tensorflow.keras.optimizers import Adam
from imblearn.over_sampling import SMOTE

db_path = 'mit-bih-arrhythmia-database-1.0.0'
name_rec = '101'
path_rec = os.path.join(db_path, name_rec)

rec_list = wfdb.get_record_list('mitdb')
x =[]
y = []
window_size = 10

for rec in rec_list:
    annotation = wfdb.rdann(os.path.join(db_path, rec), 'atr')
    peaks = annotation.sample
    intervals = np.diff(peaks)
    normalized_intervals = intervals / intervals.max()

    for i in range(len(intervals) - window_size + 1):
        window_annotations = annotation.symbol[i:i + window_size]
        window_intervals = normalized_intervals[i:i + window_size]

        abnomarl_counter = sum(1 for annotation in window_annotations if annotation == 'V')

        if all(annotation == 'N' for annotation in window_annotations) or (abnomarl_counter >= 2):
            x.append(window_intervals)
            y.append(abnomarl_counter >= 2)

x = np.array(x)
y = np.array(y)

x_train, x_test, y_train, y_test = train_test_split(x, y, test_size=0.2, random_state=42) 
smote = SMOTE()
x_train_smote, y_train_smote = smote.fit_resample(x_train, y_train)

model = Sequential([Dense(16, input_dim = window_size, activation = 'relu'),
                    Dense(8, activation = 'relu'),
                    Dense(1, activation = 'sigmoid')])

model.compile(optimizer=Adam(learning_rate=0.001), loss='binary_crossentropy', metrics=['accuracy'])

history = model.fit(x_train_smote, y_train_smote, epochs = 5, batch_size = 32, validation_split = 0.2, verbose = 1)

converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open('model.tflite', 'wb') as file:
    file.write(tflite_model)