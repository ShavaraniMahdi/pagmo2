
#define BOOST_TEST_MODULE gaco_test
#include <boost/test/included/unit_test.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <iostream>
#include <string>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/gaco.hpp>
#include <pagmo/io.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/hock_schittkowsky_71.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/minlp_rastrigin.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/serialization.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

BOOST_AUTO_TEST_CASE(construction_test)
{
    gaco user_algo{2u, 13u, 1.0, 0.0, 0.01, 1u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
    BOOST_CHECK(user_algo.get_verbosity() == 0u);
    BOOST_CHECK(user_algo.get_seed() == 23u);
    BOOST_CHECK((user_algo.get_log() == gaco::log_type{}));

    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, -0.01, 1u, 1e-7, 7u, 1000u, 1000u, 0.1, 10u, 0.9, 23u}),
                      std::invalid_argument);
    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, 0.01, 1u, 1e-7, 7u, 1000u, 1000u, -0.1, 10u, 0.9, 23u}),
                      std::invalid_argument);
    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, 0.01, 1u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, -0.1, 23u}),
                      std::invalid_argument);
    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, 0.01, 1u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 1.1, 23u}),
                      std::invalid_argument);
    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, 0.01, 3u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u}),
                      std::invalid_argument);
    BOOST_CHECK_THROW((gaco{2u, 13u, 1.0, 0.0, 0.01, 0u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u}),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(evolve_test)
{
    // Here we only test that evolution is deterministic if the
    // seed is controlled for all variants
    {
        problem prob{rosenbrock{2u}};
        population pop1{prob, 10u, 23u};
        population pop2{prob, 10u, 23u};
        population pop3{prob, 10u, 23u};

        for (unsigned int i = 1u; i < 3u; ++i) {
            gaco user_algo1{3u, 5u, 1.0, 1e9, 0.01, i, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
            user_algo1.set_verbosity(1u);
            pop1 = user_algo1.evolve(pop1);

            BOOST_CHECK(user_algo1.get_log().size() > 0u);

            gaco user_algo2{3u, 5u, 1.0, 1e9, 0.01, i, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
            user_algo2.set_verbosity(1u);
            pop2 = user_algo2.evolve(pop2);

            BOOST_CHECK(user_algo1.get_log() == user_algo2.get_log());

            gaco user_algo3{3u, 5u, 1.0, 1e9, 0.01, i, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
            user_algo3.set_verbosity(1u);
            pop3 = user_algo3.evolve(pop3);

            BOOST_CHECK(user_algo2.get_log() == user_algo3.get_log());
        }
    }
    // Here we check that the exit condition of impstop, evalstop and fstop actually provoke an exit within 300u gen
    // (rosenbrock{10} and rosenbrock{2} are used)
    {
        gaco user_algo{200u, 15u, 1.0, 0.0, 0.01, 150u, 1e-7, 7u, 1u, 1000u, 0.0, 10u, 0.9, 23u};
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        BOOST_CHECK(user_algo.get_log().size() < 200u);
    }
    {
        gaco user_algo{200u, 15u, 1.0, 0.0, 0.01, 150u, 1e-7, 7u, 1000u, 1u, 0.0, 10u, 0.9, 23u};
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 20u, 23u};
        pop = user_algo.evolve(pop);
        BOOST_CHECK(user_algo.get_log().size() < 200u);
    }
    {
        gaco user_algo{200u, 150u, 1.0, 0.0, 0.01, 150u, 1.0, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
        user_algo.set_verbosity(1u);
        problem prob{rosenbrock{2u}};
        population pop{prob, 200u, 23u};
        pop = user_algo.evolve(pop);
        BOOST_CHECK(user_algo.get_log().size() < 200u);
    }

    // We then check that the evolve throws if called on unsuitable problems
    // Integer variables problem
    BOOST_CHECK_THROW(gaco{2u}.evolve(population{problem{minlp_rastrigin{}}, 64u}), std::invalid_argument);
    // Multi-objective problem
    BOOST_CHECK_THROW(gaco{2u}.evolve(population{problem{zdt{}}, 64u}), std::invalid_argument);
    // Empty population
    BOOST_CHECK_THROW(gaco{2u}.evolve(population{problem{rosenbrock{}}, 0u}), std::invalid_argument);
    // Population size smaller than ker size
    BOOST_CHECK_THROW(gaco{2u}.evolve(population{problem{rosenbrock{}}, 60u}), std::invalid_argument);
    // Stochastic problem
    BOOST_CHECK_THROW((gaco{}.evolve(population{inventory{}, 65u, 23u})), std::invalid_argument);
    // and a clean exit for 0 generation
    population pop{rosenbrock{2u}, 10u};
    BOOST_CHECK(gaco{0u}.evolve(pop).get_x()[0] == pop.get_x()[0]);
}

BOOST_AUTO_TEST_CASE(setters_getters_test)
{
    gaco user_algo{10u, 13u, 1.0, 0.0, 0.01, 9u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
    user_algo.set_verbosity(23u);
    BOOST_CHECK(user_algo.get_verbosity() == 23u);
    user_algo.set_seed(23u);
    BOOST_CHECK(user_algo.get_seed() == 23u);
    BOOST_CHECK(user_algo.get_name().find("GACO: Ant Colony Optimization") != std::string::npos);
    BOOST_CHECK(user_algo.get_extra_info().find("Oracle parameter") != std::string::npos);
    BOOST_CHECK_NO_THROW(user_algo.get_log());
}

BOOST_AUTO_TEST_CASE(serialization_test)
{
    // Make one evolution
    problem prob{rosenbrock{2u}};
    population pop{prob, 15u, 23u};
    algorithm algo{gaco{10u, 13u, 1.0, 100.0, 0.01, 9u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u}};
    algo.set_verbosity(1u);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = boost::lexical_cast<std::string>(algo);
    auto before_log = algo.extract<gaco>()->get_log();
    // Now serialize, deserialize and compare the result.
    {
        cereal::JSONOutputArchive oarchive(ss);
        oarchive(algo);
    }
    // Change the content of p before deserializing.
    algo = algorithm{null_algorithm{}};
    {
        cereal::JSONInputArchive iarchive(ss);
        iarchive(algo);
    }
    auto after_text = boost::lexical_cast<std::string>(algo);
    auto after_log = algo.extract<gaco>()->get_log();
    BOOST_CHECK_EQUAL(before_text, after_text);
    // BOOST_CHECK(before_log == after_log); // This fails because of floating point problems when using JSON and cereal
    // so we implement a close check
    BOOST_CHECK(before_log.size() > 0u);
    for (auto i = 0u; i < before_log.size(); ++i) {
        BOOST_CHECK_EQUAL(std::get<0>(before_log[i]), std::get<0>(after_log[i]));
        BOOST_CHECK_EQUAL(std::get<1>(before_log[i]), std::get<1>(after_log[i]));
        BOOST_CHECK_CLOSE(std::get<2>(before_log[i]), std::get<2>(after_log[i]), 1e-8);
        BOOST_CHECK_CLOSE(std::get<3>(before_log[i]), std::get<3>(after_log[i]), 1e-8);
        BOOST_CHECK_EQUAL(std::get<4>(before_log[i]), std::get<4>(after_log[i]));
        BOOST_CHECK_CLOSE(std::get<5>(before_log[i]), std::get<5>(after_log[i]), 1e-8);
        BOOST_CHECK_CLOSE(std::get<6>(before_log[i]), std::get<6>(after_log[i]), 1e-8);
        BOOST_CHECK_CLOSE(std::get<7>(before_log[i]), std::get<7>(after_log[i]), 1e-8);
        BOOST_CHECK_CLOSE(std::get<8>(before_log[i]), std::get<8>(after_log[i]), 1e-8);
    }
}

BOOST_AUTO_TEST_CASE(miscellaneous_tests)
{
    problem prob{hock_schittkowsky_71{}};
    population population1{prob, 15u, 23u};
    prob.set_c_tol(1.0);
    gaco user_algo1{100u, 13u, 1.0, 1500.0, 0.01, 90u, 1e-7, 1u, 1000u, 1000u, 1000.0, 10u, 0.9, 23u};
    user_algo1.set_verbosity(1u);
    population1 = user_algo1.evolve(population1);
    population population2{prob, 15u, 23u};
    gaco user_algo2{100u, 13u, 1.0, 2700.0, 0.01, 90u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
    user_algo2.set_verbosity(1u);
    population2 = user_algo2.evolve(population2);
    population population3{prob, 15u, 23u};
    gaco user_algo3{100u, 13u, 1.0, 1500.0, 0.01, 90u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u};
    user_algo3.set_verbosity(1u);
    population3 = user_algo3.evolve(population3);
    population population4{prob, 150u, 23u};
    gaco user_algo4{10u, 130u, 1.5, 1500.0, 0.01, 9u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u}; // 1
    user_algo4.set_verbosity(1u);
    population4 = user_algo4.evolve(population4);
    population population5{prob, 150u, 23u};
    gaco user_algo5{10u, 130u, 1.5, 1500.0, 0.01, 9u, 1e-7, 7u, 1000u, 1000u, 0.0, 10u, 0.9, 23u}; // 3
    user_algo5.set_verbosity(1u);
    population5 = user_algo5.evolve(population5);
    problem prob_ros{rosenbrock{10u}};
    population population6{prob_ros, 200u, 23u};
    gaco user_algo6{20u, 150u, 1.0, 0.0, 0.0, 9u, 1e-7, 7u, 10000u, 10000u, 0.0, 10u, 0.9, 23u};
    user_algo6.set_verbosity(1u);
    population6 = user_algo6.evolve(population6);
    problem prob_cec{cec2006{1u}};
    population population7{prob_cec, 20u, 23u};
    gaco user_algo7{20u, 15u, 1.0, 1e9, 0.0, 9u, -1e7, 7u, 10000u, 10000u, 0.0, 10u, 0.9, 23u}; // 3
    user_algo7.set_verbosity(1u);
    population7 = user_algo7.evolve(population7);
    BOOST_CHECK(user_algo1.get_log().size() > 0u);
    BOOST_CHECK(user_algo2.get_log().size() > 0u);
    BOOST_CHECK(user_algo3.get_log().size() > 0u);
    BOOST_CHECK(user_algo4.get_log().size() > 0u);
    BOOST_CHECK(user_algo5.get_log().size() > 0u);
    BOOST_CHECK(user_algo6.get_log().size() > 0u);
    BOOST_CHECK(user_algo7.get_log().size() > 0u);
}
