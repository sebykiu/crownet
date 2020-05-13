#!/usr/bin/python3
# import roveranalzer
# import logging # <-- runSim.py erzeuge logfile runSim.log
import os
import shutil
import subprocess
import sys

import matplotlib.pyplot as plt
import pandas as pd
import time
import glob


def run_sim():
    os.system("chmod +x runScript.sh")
    return_code = subprocess.check_call(
        ["./runScript.sh"],
        env=os.environ,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.STDOUT,
    )


def wait_for_qoi(filename):
    filepath = f"{os.getcwd()}/results/**/{filename}"
    file = glob.glob(filepath, recursive=True)

    secs = 0
    while (len(file) < 1) and (secs <= 60):
        file = glob.glob(filepath, recursive=True)
        time.sleep(1)
        secs += 1

    if (len(file) < 1):
        file = None # prevent from infinite loop
    else:
        file = file[0]

    return file


def get_DegreeInformed_Dataframe(filepath):

    df_r = pd.read_csv(filepath, delimiter=" ", header=[0], comment="#")
    dt = 0.4  # one time frame = 0.4s
    df_r = df_r.iloc[249:-1, :]
    df_r["timeStep"] = (
            dt * df_r["timeStep"] - 100.0
    )  # information dissemination starts at 100s
    df_r.index = df_r.index - 249
    return df_r


def DegreeInformed_extract():

    filename = "DegreeInformed.txt"
    # wait for file
    filepath = wait_for_qoi(filename)
    # replace it with file extraction
    df_r = get_DegreeInformed_Dataframe(filepath)
    df_r.to_csv(os.path.join(os.path.dirname(filepath),"DegreeInformed_extract.txt"), sep=" ")

    # plot
    plt.plot(df_r.iloc[:, 0],df_r.iloc[:, 3])
    plt.axhline(y=0.95, c="r")
    plt.xlabel("Time [s] (Time = 0s : start of information dissemination)")
    plt.ylabel("Percentage of pedestrians informed [%]")
    plt.title("Information degree")
    plt.savefig(os.path.join(os.path.dirname(filepath),"InformationDegree.png"))




def Time95Informed():
    filename = "DegreeInformed.txt"
    # wait for file
    filepath = wait_for_qoi(filename)
    # replace it with file extraction
    df_r = get_DegreeInformed_Dataframe(filepath)

    dt = 0.4
    time95 = 0.0
    for perc in df_r.iloc[:, 3]:
        if perc >= 0.95:
            break
        time95 += dt

    f = open(os.path.join(os.path.dirname(filepath), "Time95Informed.txt"), "x")
    f.write(f" timeToInform95PercentAgents\n0 {time95}")
    f.close()


def post_processing():
    DegreeInformed_extract()
    Time95Informed()



def pre_processing():
    shutil.rmtree(os.path.join(os.getcwd(), "results"), ignore_errors=True)


if __name__ == "__main__":

    os.environ["ROVER_MAIN"] = "/home/christina/repos/rover-main"
    try:
        pre_processing()
        run_sim()
        post_processing()
    except:
        sys.exit(-1)
    sys.exit(0)  # ok
