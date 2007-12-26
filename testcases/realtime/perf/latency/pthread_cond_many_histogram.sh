#!/bin/bash
# thread 0 pthread_cond_broadcast() latency: 83 microseconds

awk '	{
		histogram[$5]++;
	}

	END {
		for (i in histogram) {
			print i, histogram[i];
		}
	}' | sort +0nr
