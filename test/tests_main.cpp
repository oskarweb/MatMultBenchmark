#include "test_utils.hpp"

#include <boost/program_options.hpp>
#include <gtest/gtest.h>

namespace boost_po = boost::program_options;

int main(int argc, char *argv[]) 
{
    ::testing::InitGoogleTest(&argc, argv);
    
    unsigned int rngSeed{};

    try 
    {
        boost_po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("seed", boost_po::value<unsigned int>(&rngSeed)->default_value(std::random_device{}()), "set random seed")
        ;

        boost_po::variables_map vm;        
        boost_po::store(boost_po::parse_command_line(argc, argv, desc), vm);
        boost_po::notify(vm);

        if (vm.count("help")) 
        {
            std::cout << desc << "\n";
            return 0;
        }

        GlobalRNG::init(rngSeed);

        if (vm.count("seed")) 
        {
            std::cout << "Random seed was set to " 
                 << vm["seed"].as<unsigned int>() << ".\n";
        }
    }
    catch(std::exception& e) 
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) 
    {
        std::cerr << "Exception of unknown type!\n";
    }

    return RUN_ALL_TESTS();
}