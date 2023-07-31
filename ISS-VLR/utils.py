import pandas as pd

def load_data_from_csv(path):
    return pd.read_csv(path).values
    