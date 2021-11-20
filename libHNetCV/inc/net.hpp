#include "INeuralNet.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



class HNetCV : public INeuralNet{
private:
    cv::dnn::Net m_net;
public:
    HNetCV(char *cfg, char  *weight, int gpu_id);
    HNetCV();
    ~HNetCV();

    std::vector<bbox_t> detect(image_t img, float thresh) override;
	void train(int argc,char **argv) override;
	void train_real(char *datacfg, char *cfgfile, char *weightfile, int *gpus, int ngpus, int clear, int dont_show);

};

extern "C" INeuralNet* createDetector(char *cfg, char  *weight, int gpu_id);