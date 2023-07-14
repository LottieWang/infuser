import os
import sys
import subprocess
from graphs import graphs

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))


# INFUSER_DIR = os.path.dirname(os.path.abspath(__file__))
# IM_DIR = f'{INFUSER_DIR}/../../IM'


def run_infuser(graph, w, folder):
    print(f'running infuser on {graph}')
    g = graph.split('/')[-1]
    file_out = f'{INFUSER_DIR}/{folder}/{g[:-4]}.txt'
    mem_out = f'{INFUSER_DIR}/{folder}/{g[:-4]}_mem.txt'
    # for i in range(4,14):
    #     R = 1<<i
    R=256
    subprocess.call(f'echo R {R} >> {file_out}',shell=True)
    # command = f'{INFUSER_DIR}/bin/infuser -M HyperFuser -K 200 -R {R} -p {w} {graph}'
    command = f'{INFUSER_DIR}/../bin/infuser -M infuser -K 100 -R {R} -p {w} {graph}'
    # subprocess.call(f'numactl -i all {command} >> {file_out}', shell=True)
    subprocess.call(
        f'/usr/bin/time -v numactl -i all {command} 1>> {file_out} 2>> {mem_out}', shell=True)

def run_infuser_scale(graph,  w, folder):
    g = graph.split('/')[-1]
    file_out = f'{INFUSER_DIR}/{folder}/{g[:-4]}.txt'
    mem_out = f'{INFUSER_DIR}/{folder}/{g[:-4]}_mem.txt'
    R=256
    command = f'{INFUSER_DIR}/../bin/infuser -M infuser -K 100 -R {R} -p {w} {graph}'
    # n_threads = [192, 96, 48, 24, 12, 8, 4, 2, 1]
    # cores = ['0-191', '0-95', '0-95:2', '0-95:4', '0-47:4', '0-31:4', '0-15:4', '0-7:4','0-3:4']
    # n_threads = [192, 96, 48, 24, 16, 8, 4]
    # cores = ['0-191', '0-95', '0-95:2', '0-95:4', '0-63:4', '0-31:4', '0-15:4']
    n_threads = [2,1]
    cores = ['0-7:4','0-3:4']
    # n_threads = [4]
    # cores = ['0-15:4']
    for i in range(len(n_threads)):
        numa = "numactl -i all"
        n_thread = n_threads[i]
        core = cores[i]
        print(f'running infuser on {graph} n_thread {n_thread} cores {core}')
        if (n_thread == 1):
            numa = ''
        subprocess.call(f'echo n_threads {n_thread} cores {core} R {R} >> {file_out}',shell=True)
        subprocess.call(
            f'OMP_NUM_THREADS={n_thread} taskset -c {core} {numa} {command} 1>> {file_out} 2>> {mem_out}', shell=True)



def run_infuser_all(folder):
    for g in graph.get_small_graphs():
        run_infuser(g,0.02,folder)
    for g in graph.get_large_graphs():
        run_infuser(g, 0.02, folder)
    for g in graph.get_road_graphs():
        run_infuser(g, 0.2, folder)
    for g in graph.get_knn_graphs():
        run_infuser(g,0.2, folder)
    # for g in graph.get_grid_graphs():
    #     run_infuser(g,0.2,folder)

def run_infuser_scale_all(folder):
    for g in graph.get_small_graphs():
        run_infuser_scale(g,0.02,folder)
    for g in graph.get_large_graphs():
        run_infuser_scale(g,0.02,folder)
    for g in graph.get_road_graphs():
        run_infuser_scale(g,0.2,folder)
    for g in graph.get_knn_graphs():
        run_infuser_scale(g,0.2,folder)
    # for g in graph.get_grid_graphs():
    #     run_infuser_scale(g,0.2,folder)



if __name__ == '__main__':
    subprocess.call("export LD_PRELOAD=/usr/local/lib/libjemalloc.so",shell=True)
    subprocess.call(f'mkdir -p {CURRENT_DIR}/../logs', shell=True)
    infuser = f'{CURRENT_DIR}/../bin/infuser'
    # general_cascade = f'{CURRENT_DIR}/../general_cascade'
    if not os.path.exists(infuser):
        print('Please build by `make -j` first.')
        assert (0)
    for graph, url, w, GRAPH_DIR in graphs:
        graph_file = f'{GRAPH_DIR}/{graph}.bin'
        if not os.path.exists(graph_file):
            print(f'\nWarning: {graph} does not exists')
            # continue
        seeds_file = f'{CURRENT_DIR}/../logs/{graph}.txt'
        mem_file = f'{CURRENT_DIR}/../logs/{graph}_mem.txt'
        print(f'\nRunning infuser on {graph_file}')
        # test uniform distribution
        if (w == 0.02):
            u1 = 0
            u2=0.1
        if (w == 0.2):
            u1 = 0.1
            u2 = 0.3
        command = f'{infuser} -M infuser -K 100 -R 256 -ua {u1} -ub {u2} {graph_file}'
        print(f'Seeds saved to {seeds_file}')
        subprocess.call(
            f'/usr/bin/time -v numactl -i all {command} 1>> {seeds_file} 2>> {mem_file}', shell=True)
        # subprocess.call(cmd, shell=True)
        command = f'{infuser} -M infuser -K 100 -R 256 -WIC {graph_file}'
        print(f'Seeds saved to {seeds_file}')
        subprocess.call(
            f'/usr/bin/time -v numactl -i all {command} 1>> {seeds_file} 2>> {mem_file}', shell=True)
        