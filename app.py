import streamlit as st
import pandas as pd
import pickle

# Load pre-trained model and preprocessor
with open("model.pkl", "rb") as f:
    model = pickle.load(f)

with open("preprocessor.pkl", "rb") as f:
    preprocessor = pickle.load(f)

# Page Title
st.title("🔧 5G Network Resource Allocator")
st.write("Upload a CSV file with input metrics to get allocated resources.")

# File Upload
uploaded_file = st.file_uploader("Upload your slice_input_metrics.csv", type=["csv"])

if uploaded_file is not None:
    df = pd.read_csv(uploaded_file)
    st.subheader("📥 Uploaded Data")
    st.dataframe(df)

    try:
        # Preprocess and Predict
        processed_data = preprocessor.transform(df)
        prediction = model.predict(processed_data)

        # Create output DataFrame
        output_features = ['Allocated_Bandwidth', 'Allocated_CPU', 'Allocated_Memory']
        result_df = pd.DataFrame(prediction, columns=output_features)

        # Combine input + output
        # full_df = pd.concat([df, result_df], axis=1)

        # Display Results
        st.subheader("📊 Allocated Resources")
        st.dataframe(result_df)

        # Download Button
        csv = result_df.to_csv(index=False).encode('utf-8')
        st.download_button("📥 Download Results as CSV", csv, "resource_allocation_output.csv", "text/csv")

    except Exception as e:
        st.error(f"Error during prediction: {e}")
else:
    st.info("Awaiting file upload...")

