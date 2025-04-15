import pickle
import pandas as pd
with open("model.pkl", "rb") as f:
    model = pickle.load(f)


with open("preprocessor.pkl", "rb") as f:
    preprocessor = pickle.load(f)
df=pd.read_csv("slice_input_metrics.csv")
processed_data=preprocessor.transform(df)
prediction=model.predict(processed_data)
print(prediction)