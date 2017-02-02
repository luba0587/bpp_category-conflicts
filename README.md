# bpp_category-conflicts
bin packing as a subproblem of vehicle routing problem, in which the are incompatibilities between product categories
this code was made for a class (USP-PTR5001) between oct 2016 and jan 2017

undefine M_GRANDE if you don't want to run the optimization with Gurobi (for which a license is required)

Instances were created based on Delorme et al. (2016) -> we adopted 6 cargo types
Input files:
- instances' names are given by ArquivosIntancias-saidas.txt (edit it to run more/less instances, or to change input files paths)
- instances' data are given by files on folders Instancias and InstanciasNR
- compatibility between categories is given by Reduced_set.vrp (1 indicates items compatibility and 0 otherwise)
