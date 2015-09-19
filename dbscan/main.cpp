#include "hpdbscan.h"

#include <cstring>
#include <iostream>
#include <omp.h>
#include <string>
struct Parameters
{
    std::string file;
    std::string out;
    size_t      minPoints;
    size_t      threads;
    float       epsilon;
    
    Parameters() : 
        file("data.csv"), 
        out("out.csv"),
        minPoints(0L),
        threads(omp_get_max_threads()),
        epsilon(0.0f)
    {}
};

void usage(char* program)
{
    std::cerr << "Usage: " << program << " -m minPoints -e epsilon [OPTIONS] [FILE=data.csv]" << std::endl
              << "    Format : One data point per line, whereby each line contains the space-seperated values for each dimension '<dim 1> <dim 2> ... <dim n>'" << std::endl
              << "    -m minPoints : DBSCAN parameter, minimum number of points required to form a cluster, postive integer, required" << std::endl
              << "    -e epsilon   : DBSCAN parameter, maximum neighborhood search radius for cluster, positive floating point, required" << std::endl
              << "    -o output    : Processing parameter, path to output file, string, defaults to out.csv" << std::endl
              << "    -t threads   : Processing parameter, the number of threads to use, positive integer, defaults to number of cores" << std::endl
              << "    -h help      : Show this help message" << std::endl
              << "    Output : A copy of the input data points plus an additional column containing the cluster id, the id 0 denotes noise" << std::endl;
}

inline ssize_t stoll(const char* optarg)
{
    size_t  length;
    ssize_t parsed;
    
    try
    {
        parsed = std::stoll(optarg, &length);
    }
    catch (const std::logic_error& e)
    {
        parsed = 0L;
    }
    
    return length < strlen(optarg) ? 0L : parsed;
}

inline float stof(const char* optarg)
{
    size_t length;
    float  parsed;
    
    try
    {
        parsed = std::stof(optarg, &length);
    }
    catch (const std::logic_error& e)
    {
        parsed = 0.0f;
    }
    
    return length < strlen(optarg) ? 0.0f : parsed;
}

Parameters parse(int argc, char** argv)
{
    Parameters parameters;
    int i;
    int errors = 0;
    
    for (i = 1; i < argc - 1; ++i)
    {
        std::string option = argv[i];
        
        if (option == "-h")
        {
            usage(argv[0]);
            std::exit(EXIT_SUCCESS);
        }
        else if (option == "-m")
        {
            ++i;
            if (i >= argc)
            {
                ++errors;
                std::cerr << "no value for minPoints provided" << std::endl;
                continue;
            }
            
            const char* optarg = argv[i];
            ssize_t minPoints = stoll(optarg);
            if (minPoints <= 0L)
            {
                parameters.minPoints = 1L;
                ++errors;
                std::cerr << "minPoints needs to be a positive integer number, but was " << optarg << std::endl;
            }
            else
                parameters.minPoints = (size_t) minPoints;
        }
        else if (option == "-e")
        {
            ++i;
            if (i >= argc)
            {
                ++errors;
                std::cerr << "no value for epsilon provided" << std::endl;
                continue;
            }
            
            const char* optarg = argv[i];
            float epsilon = stof(optarg);
            if (epsilon <= 0.0f)
            {
                parameters.epsilon = 1.0f;
                ++errors;
                std::cerr << "epsilon needs to be a positive floating point number, but was " << optarg << std::endl;
            }
            else
            {
                parameters.epsilon = epsilon;
            }
        }
        else if (option == "-o")
        {
            ++i;
            if (i >= argc)
            {
                ++errors;
                std::cerr << "no output path provided" << std::endl;
                continue;
            }
            parameters.out = argv[i];
        }
        else if (option == "-t")
        {
            ++i;
            if (i >= argc)
            {
                ++errors;
                std::cerr << "no thread count provided" << std::endl;
                continue;
            }
            
            const char* optarg = argv[i];
            ssize_t threads = stoll(optarg);
            if (threads <= 0L)
            {
                parameters.threads = omp_get_max_threads();
                ++errors;
                std::cerr << "threads needs to be a positive integer number, but was " << optarg << std::endl;
            }
            else
                parameters.threads = (size_t) threads;
        }
        else if (option[0] == '-')
        {
            ++errors;
            std::cerr << "unknown option " << option << std::endl;
            break;
        }
        else
        {
            parameters.file = option;
        }
    }
    
    if ((argc - i) != 1)
    {
        ++errors;
        std::cerr << "Please provide exactly one input file" << std::endl;
    }
    else
    {
        parameters.file = argv[i];
    }
    
    if (errors)
    {
        std::cerr << std::endl;
        usage(argv[0]);
        std::exit(EXIT_SUCCESS);
    }
    
    return parameters;
}



int read(std::string filename, float** points, int& dim)
{
	std::cout << "HERE" << std::endl;

	*points = new float[10];
	(*points)[0]=1;
	(*points)[1]=2;
	(*points)[2]=3;
	(*points)[3]=4;
	(*points)[4]=5;
	(*points)[5]=6;
	(*points)[6]=7;
	(*points)[7]=8;
	(*points)[8]=9;
	(*points)[9]=10;
	
	dim = 2;
	return 5;
}

int main(int argc, char** argv)
{    
    Parameters parameters = parse(argc, argv);
    double start = omp_get_wtime();
    
    omp_set_num_threads(parameters.threads);
	
	int dim;
	float* data;
	int npoints = read(parameters.file, &data, dim);

    HPDBSCAN scanner(data,npoints,dim);
    scanner.scan(parameters.epsilon, parameters.minPoints);
    
    std::cout << "Took: " << (omp_get_wtime() - start) << "s" << std::endl;
    
    return 0;
}
