This folder holds the code necessary to cluster the poker hands at the flop and turn, based on their hand strenght distribution at the river. 

To get the clusters, the first step is to run '''run_flop.sh''' or hopefully its slurm version '''sbatch_flop_dist.sh'''. This will result in a file or number of files depending on how many threads the script used. Just type cat '''cat distributions_flop_* > distributions_flop.csv''' to get a single file if it was run with more than one thread. 

This file holds a large number of distributions (25989600 for the flop, 305377800 for the turn). Holding these many distributions in memory is pointless since many are identical. To overcome this run '''run_canonicalise.sh''',  this will write only canonical distributions to an output csv file, reducing the number of distributions by a factor of ~20. 

Finally to get the clusters run '''run_clusters.sh''' which will return the csv file with a cluster associated to each canonical hand. Note that the number of cluster of choice is not trivial and we also don't really know what we are doing.

To get the clusters for the turn the procedure is the same, just comment out the flop parameters in '''canonicalise_distributions.cpp''' and '''cluster.cpp''', and change the names of the files. Good luck!