#include "network.h"

#include <vector>
#include <tuple>


std::tuple<int,void*> get_convolutional_weights(layer l);

std::tuple<int,void*> get_connected_weights(layer l);

std::tuple<int,void*> get_batchnorm_weights(layer l);

std::vector<std::tuple<int,void*>> get_weights_upto(network net, int cutoff);

std::vector<std::tuple<int,void*>> get_weights(network net);
void set_weights(network net, std::vector<std::tuple<int,void*>> weights);

void write_weights_mem(std::vector<std::tuple<int,void*>> &weights, std::string filename);

void rgb_to_hfm(image img);