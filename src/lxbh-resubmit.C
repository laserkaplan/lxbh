/*
 * 'lxbh-resubmit' by Laser
 * To clean up failed lxbatch EventLoop jobs and resubmit them easily
 * Mandatory arguments:
 *      -d, --submitDir : submitDir made by EventLoop
 *      -n, --number    : segment number
 *      -q, --queue     : queue to submit to
 * Optional arguments:
 *      -f, --force     : force running (remove all files from job and resubmit cleanly)
 *      -h, --help      : show the help dialog
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>

bool parseArgs(int argc, const char **argv, std::map<std::string, std::string> &args);
void help();

int main(int argc, const char **argv) {
    // parse arguments
    std::map<std::string, std::string> args;
    if (!parseArgs(argc, argv, args)) {
        help();
        std::cout << "Error parsing arguments!  Exiting!" << std::endl;
        return 1;
    }

    // make sure mandatory arguments are there
    if (args["submitDir"].empty() || args["number"].empty() || args["queue"].empty()) {
        help();
        std::cout << "Missing required argument!  Exiting!" << std::endl;
        return 2;
    }

    // make sure submitDir is NOT relative
    if (args["submitDir"][0] != '/') {
        std::cout << "submitDir MUST be an absolute path! (/afs/cern.ch/...)" << std::endl;
        return 3;
    }

    // check if we can modify the submitDir
    if (access(args["submitDir"].c_str(), W_OK) != 0) {
        std::cout << "Cannot write to submit directory!  Exiting!" << std::endl;
        return 4;
    }

    // check if fail files exists
    if (access((args["submitDir"] + "/fetch/fail-" + args["number"]).c_str(), F_OK) != 0) {
        if (!args["force"].compare("0")) {
            std::cout << "Fail file does not exist and you did not specify -f!  Exiting!" << std::endl;
            return 5;
        }
    }

    // delete files associated with job
    system(("rm " + args["submitDir"] + "/fetch/done-" + args["number"]).c_str());
    system(("rm " + args["submitDir"] + "/fetch/fail-" + args["number"]).c_str());
    if (!args["force"].compare("1")) {
        system(("rm " + args["submitDir"] + "/fetch/completed-" + args["number"]).c_str());
        std::ifstream segments((args["submitDir"] + "/submit/segments").c_str());
        if (segments.good()) {
            std::string seg, name;
            while (segments >> seg >> name) {
                if (!args["number"].compare(seg)) {
                    system(("rm " + args["submitDir"] + "/fetch/hist-" + name + ".root").c_str());
                    break;
                }
            }
        }
    }

    // resubmit the job
    std::string command = "bsub";
    command += " -q " + args["queue"];
    command += " -L /bin/bash";
    command += " 'export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh && ";
    command += args["submitDir"];
    command += "/submit/run ";
    command += args["number"];
    command += "'";
    std::cout << command << std::endl;
    system(command.c_str());

    return 0;
}

bool parseArgs(int argc, const char **argv, std::map<std::string, std::string> &args) {
    // reset args map
    args["submitDir"] = "";
    args["number"]    = "";
    args["queue"]     = "";
    args["force"]     = "0";

    // argument loop
    int ia = 1;
    while (ia < argc) {
        std::string arg1 = argv[ia];
        if (!(arg1.compare("-d")) || !(arg1.compare("--submitDir"))) {
            ia++;
            if (ia == argc) {std::cout << "Argument does not have value: " << arg1 << std::endl; return false;}
            std::string arg2 = argv[ia];
            args["submitDir"] = arg2;
        }
        else if (!(arg1.compare("-n")) || !(arg1.compare("--number"))) {
            ia++;
            if (ia == argc) {std::cout << "Argument does not have value: " << arg1 << std::endl; return false;}
            std::string arg2 = argv[ia];
            args["number"] = arg2;
        }
        else if (!(arg1.compare("-q")) || !(arg1.compare("--queue"))) {
            ia++;
            if (ia == argc) {std::cout << "Argument does not have value: " << arg1 << std::endl; return false;}
            std::string arg2 = argv[ia];
            args["queue"] = arg2;
        }
        else if (!(arg1.compare("-f")) || !(arg1.compare("--force"))) {
            args["force"] = "1";
        }
        else {
            std::cout << "Error parsing argument: " << arg1 << std::endl;
            return false;
        }
        ia++;
    }

    return true;
}

void help() {
    std::cout << "'lxbh-resubmit' by Laser" << std::endl;
    std::cout << "To clean up failed lxbatch EventLoop jobs and resubmit them easily" << std::endl;
    std::cout << "Mandatory arguments:" << std::endl;
    std::cout << "     -d, --submitDir : submitDir made by EventLoop" << std::endl;
    std::cout << "     -n, --number    : segment number" << std::endl;
    std::cout << "     -q, --queue     : queue to submit to" << std::endl;
    std::cout << "Optional arguments:" << std::endl;
    std::cout << "     -f, --force     : force running (remove all files from job and resubmit cleanly)" << std::endl;
    std::cout << "     -h, --help      : show the help dialog" << std::endl;
}
