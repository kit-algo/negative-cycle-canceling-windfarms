# Negative Cycle Canceling for the Windfarm Cabling Problem

This is a C++ implementation of an algorithm for the Windfarm Cabling Problem (WCP) based on Negative Cycle Canceling (NCC). The algorithm was developed at [Karlsruhe Institute of Technology (KIT)](http://www.kit.edu) in the [group of Prof. Dorothea Wagner](https://i11www.iti.kit.edu/). The implementation is a reworked version of the code proposed in the following publications:

* *Towards Negative Cycle Canceling in Wind Farm Cable Layout Optimization*
  Sascha Gritzbach, Torsten Ueckerdt, Dorothea Wagner, Franziska Wegner, and Matthias Wolf.
  In *Proceedings of The 7th DACH+ Conference on Energy Informatics*, Energy Informatics Volume 1, Supplement 1, 2018.
  [doi](https://doi.org/10.1186/s42162-018-0030-6)
* *Engineering Negative Cycle Canceling for Wind Farm Cabling*
  Sascha Gritzbach, Torsten Ueckerdt, Dorothea Wagner, Franziska Wegner, and Matthias Wolf.
  In *Proceedings of the 27th Annual European Symposium on Algorithms (ESA '19)*, Volume 144 of Leibniz International Proceedings in Informatics, Schloss Dagstuhl -- Leibniz-Zentrum für Informatik, 2019.
  [doi](https://doi.org/10.4230/LIPIcs.ESA.2019.55) [arXiv](https://arxiv.org/abs/1908.02129)
* *Negative Cycle Canceling with Neighborhood Heuristics for the Wind Farm Cabling Problem*
  Sascha Gritzbach, Dorothea Wagner, and Matthias Wolf.
  In *Proceedings of the Eleventh ACM International Conference on Future Energy Systems (e-Energy '20)*, Association for Computing Machinery, 2020.
  [doi](https://doi.org/10.1145/3396851.3397754)

More comprehensive information, as well as a link to the respective evaluation data, can be found in a doctoral thesis based on the aforementioned publications:

* *Cable Layout Optimization Problems in the Context of Renewable Energy Sources*
  Sascha Gritzbach.
  PhD Thesis, Karlsruhe Institute of Technology (KIT), 2023.
  [doi](https://doi.org/10.5445/IR/1000158746)

## Prerequisites
Before you start, make sure to have the following at hand:

* a somewhat recent C and C++ compiler (we have used gcc/g++ 7.5.0),
* the [Open Graph Drawing Framework (OGDF)](https://ogdf.uos.de) snapshot 2017-07-23 already compiled as an in-source build,
* QT5 and
* the benchmark instances from the paper *A Simulated-Annealing-Based Approach for Wind Farm Cabling* by Sebastian Lehmann, Ignaz Rutter, Dorothea Wagner, and Franziska Wegner. [(Link to Paper)](https://doi.org/10.1145/3077839.3077843) [(Link to Instances)](https://i11www.iti.kit.edu/projects/windfarmcabling/index)

## Building
We recommend that you use the script ``useThisToBuild.sh`` from the ``scripts`` folder. Before you run this script, make sure to familiarize yourself with the guidance provided in the beginning of the script. In particular, you will have to specify some file paths first.

## Usage
Upon building, you will find an executable called ``windfarmCabling`` in the ``bin`` subdirectory of your build directory. This executable accepts various command-line arguments to specify settings for the NCC algorithm. If you invoke the executable without arguments (or with the ``-h``/``--help`` flags), you will be given an overview of the available options with their default values. The options are as follows:

* Choice of Algorithm: ``-a <NameOfAlgorithm>``/``--algorithm <NameOfAlgorithm>`` Use either ``NCC`` for the standalone NCC-algorithm as in the DACH- and ESA-papers or ``EscapingNCC`` for the NCC-algorithm embedded into an Iterated Local Search as in the e-Energy-paper.
* Choice of Instance: ``-i <FileName>``/``--inputFile <FileName>`` Specify on which instance you want to run the algorithm. You can specify full paths or paths relative to the current working directory. Note that due to the use of regular expressions in the code with the goal to provide output information, the specified ``FileName`` (including the specified path) needs at least two occurences of a dash ``-`` followed by a number. The first two occurrences are used as the testset and instance number. If that requirement is not met, non-meaningful numbers are inserted. For information on the format that custom instances must adhere to, refer to the paper in the ``Prerequisites`` section of this Readme.
* Output Directory: ``-o <Path>``/``--output <Path>`` Specify the path where any output is written to. This path may be given relative to the current working directory or as a full path. In either case, make sure to include a path separator at the end.
* Time Limit: ``-z <LimitInSeconds>``/``--time <LimitInSeconds>`` Specify the limit on the maximum running time in seconds of the algorithm. If ``EscapingNCC`` is chosen as the algorithm, it is advisable to specify at least one of time or iteration limit (see below).
* Iteration Limit: ``-l <Limit>``/``--iterations <Limit>`` Specify the maximum number of iterations used when ``EscapingNCC`` is chosen as the algorithm. A limit of 1 corresponds to the standalone NCC-algorithm (if you want that, rather use ``NCC`` in the ``--algorithm`` option) and a limit of 0 corresponds to computing an initial flow only. It is advisable to specify at least one of iteration or time limit (see above) when using the ``EscapingNCC`` algorithm option.
* Initialization Strategy: ``--init <Strategy>`` Specify the initialization strategy used in the NCC algorithm. The input must be of the form ``(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)``. For explanations on how these strategies work, kindly refer to the aforementioned ESA-paper.
* Delta Strategy: ``--delta <Strategy>`` Specify the delta strategy used in the NCC algorithm. The input must be of the form ``(STAY)?(INC|DEC|INCDEC|RANDOM)``. For explanations on how these strategies work, kindly refer to the aforementioned ESA-paper.
* Detection Strategy: ``--detect <Strategy>`` Specify the labels used in the Bellman-Ford algorithm. ``Strategy`` must be either ``BF`` for the standard Bellman-Ford algorithm or ``STFBF`` (read as: side-trip-free Bellman-Ford) for the Bellman-Ford algorithm using two labels per vertex to avoid 2-cycles as explained in the ESA-paper. Most probably, you will want to use ``STFBF``.
* Weights of Escaping Strategies: ``--escapeLeaves <Weight>``/``--escapeFreeCables <Weight>``/``--escapeBonbon <Weight>`` Specify the weight (as a positive integer) with which an escaping strategy is chosen at the end of one iteration in the Iterated Local Search. The weight yields a probability by means of the ratio of the specific weight of a strategy to the total weight across all escaping strategies. If ``EscapingNCC`` is chosen in the ``--algorithm`` option, at least one escaping strategy with a positive weight needs to be chosen. For further information on the probability and on how the escaping strategies work, kindly refer to the aforementioned e-Energy-paper.
* Random Seed: ``--seed <integer>`` Specify an integer to serve as a random seed for the delta strategies ``Random`` and ``StayRandom`` as well as for the order in which escaping strategies are applied.
* Granularity of Output: Use the following options to specify the level of detail in which information from the algorithm is provided. Make sure to use at least some of them, otherwise you will receive no output at all.
    - ``--verbose`` Prints real-time information to standard out.
    - ``--print-solution`` Upon termination, write a file with the best solution from the algorithm as flow values into a graph structure.
    - ``--print-summary`` Upon termination, append a line containing the most important pieces of information from the algorithm into a shared file.
    - ``--print-details`` Upon termination, write a file with step-by-step information from the algorithm.
  The structure of the files should be pretty self-explanatory. We only mention two things about the status of the algorithms. The status refers to the return codes shown in the file ``Auxiliary/ReturnCodes.h``. In the file with full details obtained from a run of ``EscapingNCC``, the status of an escaping strategy is written into the ``delta`` column.
