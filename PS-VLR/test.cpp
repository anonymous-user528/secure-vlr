#include "src/utils/hexl_utils.h"
#include "src/client/client.hpp"
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include "src/config/config.h"
#include "src/network/multi_party_player.hpp"
#include "src/models/psvlr.h"

using namespace std;

int main(int argc, char *argv[]) {
    std::size_t my_pid, n_players;
    std::string network_file, data_file;

    srand(time(0));

    namespace po = boost::program_options;
    po::options_description description("Usage:");
    description.add_options()
        ("help,h", "display this help message")
        ("version,v", "display the version number")
        ("client-id", po::value<std::size_t>(&my_pid), "current client id")
        ("client-num", po::value<std::size_t>(&n_players), "total client num")
        ("network-file", po::value<std::string>(&network_file), "network file used")
        ("data-file", po::value<std::string>(&data_file), "dataset used for the task");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    std::size_t n_threads = n_players - 1;
    ConfigFile config_file(network_file);
    std::string portString, ipString;
    std::vector<boost::asio::ip::tcp::endpoint> endpoints;
    mplayerid_t parties;
    for (int i = 0; i != n_players; ++i) {
        portString = "party_" + std::to_string(i) + "_port";
        ipString = "party_" + std::to_string(i) + "_ip";
        if(i == my_pid) {
            endpoints.emplace_back(boost::asio::ip::address_v4(), std::stoi(config_file.value("",portString)));
        }
        else {
            endpoints.emplace_back(boost::asio::ip::address::from_string(config_file.value("",ipString)), std::stoi(config_file.value("",portString)));
            parties.insert(i);
        }
        std::cout << i << ": (" << config_file.value("",ipString) << ", " << config_file.value("",portString) << ")" << std::endl;
    }
    
    network::PlainMultiPartyPlayer player(my_pid, n_players);
    player.run(n_threads);
    player.connect(endpoints);
    constexpr size_t K = 128, N = K, D = 12;
    Semi2kContext<K> sc(&player, parties, my_pid, time(0) + my_pid);
    FSemi2kContext<N, D> fsc(sc);

    bool has_label = (my_pid == SUPER_CLIENT_ID);

    Client client(my_pid, n_players, has_label, fsc, data_file, parties, my_pid, &player);

    PSVLR model(client, 512);

    model.share_data();

    client.initialize_keys(2, 50, 4096);

    std::vector<std::vector<std::vector<double>>> u, u_transpose;
    u.resize(4);
    u_transpose.resize(4);
    for(int i = 0; i != u.size(); ++i){
        u[i].resize(model.batchsize);
        for(int j = 0; j != u[i].size(); ++j){
            u[i][j].resize(model.shared_data[0].size());
        }
        u_transpose[i].resize(model.shared_data[0].size());
        for(int j = 0; j != u_transpose[i].size(); ++j){
            u_transpose[i][j].resize(model.batchsize);
        }
        
    }

    client.generate_matrix_triple(u, 1, 4, model.batchsize, model.shared_data[0].size());
    client.generate_matrix_triple(u_transpose, 1, 4, model.shared_data[0].size(), model.batchsize);

    model.train(1);


}


