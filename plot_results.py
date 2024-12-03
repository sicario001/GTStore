#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

def plot_single_client():
    print("Plotting single client results...")
    replicas = []
    throughputs = []
    put_throughputs = []
    get_throughputs = []
    
    with open('single_client_results.txt', 'r') as f:
        for line in f:
            r, t, pt, gt = map(float, line.strip().split())
            replicas.append(int(r))
            throughputs.append(t)
            put_throughputs.append(pt)
            get_throughputs.append(gt)
    
    plt.figure(figsize=(10, 6))
    x = np.arange(len(replicas))
    width = 0.25
    
    plt.bar(x - width, put_throughputs, width, label='PUT', color='skyblue')
    plt.bar(x, get_throughputs, width, label='GET', color='lightgreen')
    plt.bar(x + width, throughputs, width, label='Overall', color='salmon')
    
    plt.xlabel('Number of Replicas')
    plt.ylabel('Throughput (Ops/s)')
    plt.title('GTStore Single Client Performance\n(PUT vs GET vs Overall)')
    plt.xticks(x, replicas)
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.legend()
    
    # Add value labels on top of each bar
    for i, v in enumerate(put_throughputs):
        plt.text(i - width, v, f'{v:.0f}', ha='center', va='bottom')
    for i, v in enumerate(get_throughputs):
        plt.text(i, v, f'{v:.0f}', ha='center', va='bottom')
    for i, v in enumerate(throughputs):
        plt.text(i + width, v, f'{v:.0f}', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig('single_client_throughput.png')
    plt.close()

def plot_concurrent():
    print("Plotting concurrent client results...")
    replicas = []
    threads = []
    throughputs = []
    
    with open('throughput_results.txt', 'r') as f:
        for line in f:
            r, t, tp = map(float, line.strip().split())
            replicas.append(int(r))
            threads.append(int(t))
            throughputs.append(tp)
    
    plt.figure(figsize=(10, 6))
    plt.bar(replicas, throughputs, color='skyblue')
    plt.xlabel('Number of Replicas')
    plt.ylabel('Throughput (Ops/s)')
    plt.title(f'GTStore Concurrent Performance\n({threads[0]} clients, mixed read/write)')
    plt.xticks(replicas)
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    
    # Add value labels on top of each bar
    for i, v in enumerate(throughputs):
        plt.text(replicas[i], v, f'{v:.0f}', ha='center', va='bottom')
    
    plt.savefig('concurrent_throughput.png')
    plt.close()

def plot_loadbalance():
    try:
        node_counts = {}
        with open('loadbalance_results.txt', 'r') as f:
            for line in f:
                node, count = line.strip().split()
                node_counts[node] = int(count)
        
        # Sort nodes by their ID
        nodes = sorted(node_counts.keys())
        counts = [node_counts[node] for node in nodes]
        
        plt.figure(figsize=(10, 6))
        plt.bar(range(len(nodes)), counts)
        plt.xlabel('Storage Node')
        plt.ylabel('Number of Keys')
        plt.title('Key Distribution Across Storage Nodes')
        plt.xticks(range(len(nodes)), nodes, rotation=45)
        plt.grid(True, axis='y', linestyle='--', alpha=0.7)
        
        # Add value labels on top of each bar
        for i, v in enumerate(counts):
            plt.text(i, v, str(v), ha='center', va='bottom')
        
        plt.tight_layout()
        plt.savefig('loadbalance.png')
        plt.close()
    except FileNotFoundError:
        print("No load balance results found")

if __name__ == "__main__":
    try:
        plot_single_client()
    except FileNotFoundError:
        print("No single client results found")
    
    try:
        plot_concurrent()
    except FileNotFoundError:
        print("No concurrent results found")
    
    try:
        plot_loadbalance()
    except FileNotFoundError:
        print("No load balance results found")
