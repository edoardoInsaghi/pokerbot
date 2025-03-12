import pandas as pd
from sklearn.cluster import KMeans
import numpy as np

print('Loading data...')

# Load the dataset
data = pd.read_csv('flop_data.csv', header=None)

identifiers = data.iloc[:, 0]  # First column as identifiers
features = data.iloc[:, 1:].values  # Remaining columns as features

features = np.array(features, dtype=np.float32)

print('Clustering data...')
n_clusters = 128  # Define the number of clusters you want
kmeans = KMeans(n_clusters=n_clusters)
clusters = kmeans.fit_predict(features)

# Create a dataframe with identifiers and cluster assignments
results = pd.DataFrame({
    'id': identifiers,
    'cluster': clusters
})

print('Saving results...')
results.to_csv('kmeans_clusters.csv', index=False)
