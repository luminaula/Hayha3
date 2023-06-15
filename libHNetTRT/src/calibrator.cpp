/**
MIT License

Copyright (c) 2018 NVIDIA CORPORATION. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*
*/

#include "calibrator.h"
#include <fstream>
#include <iostream>
#include <iterator>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"


Int8EntropyCalibrator::Int8EntropyCalibrator(const uint32_t& batchSize, const std::string& calibImages,
                                             const std::string& calibImagesPath,
                                             const std::string& calibTableFilePath,
                                             const uint64_t& inputSize, const uint32_t& inputH,
                                             const uint32_t& inputW, const std::string& inputBlobName) :
    m_BatchSize(batchSize),
    m_InputH(inputH),
    m_InputW(inputW),
    m_InputSize(inputSize),
    m_InputCount(batchSize * inputSize),
    m_InputBlobName(inputBlobName),
    m_CalibTableFilePath(calibTableFilePath),
    m_ImageIndex(0)
{
    if (!fileExists(m_CalibTableFilePath, false))
    {
        m_ImageList = loadImageList(calibImages, calibImagesPath);
        m_ImageList.resize(static_cast<int>(m_ImageList.size() / m_BatchSize) * m_BatchSize);
        //std::random_shuffle(m_ImageList.begin(), m_ImageList.end(),
        //                    [](int i) { return rand() % i; });
    }

    NV_CUDA_CHECK(cudaMalloc(&m_DeviceInput, m_InputCount * sizeof(float)));
}

Int8EntropyCalibrator::~Int8EntropyCalibrator() { NV_CUDA_CHECK(cudaFree(m_DeviceInput)); }

float *Int8EntropyCalibrator::loadImages(std::vector<std::string> names){
    float *batchData = new float[m_InputCount];

    int x,y,c;

    for(int i=0; i<names.size();i++){
        unsigned char *data = stbi_load(names.at(i).c_str(),&x,&y,&c,3);
        unsigned char *resized = new unsigned char[m_InputW * m_InputH * 3];

        stbir_resize_uint8(data,x,y,0,resized,m_InputW,m_InputH,0,c);

        float *r, *g, *b;

        r = batchData + (i * m_InputW * m_InputH * 3);
        g = r + m_InputW * m_InputH;
        b = g + m_InputW * m_InputH;

        for(int j=0;j<m_InputH;j++){
            for(int k=0;k<m_InputW;k++){
                *r++ = (float)resized[j * m_InputW * 3 + k * 3] / 255.0;
                *g++ = (float)resized[j * m_InputW * 3 + k * 3 + 1] / 255.0;
                *b++ = (float)resized[j * m_InputW * 3 + k * 3 + 2] / 255.0;
            }
        }


        stbi_image_free(data);
        delete[] resized;
    }


    return batchData;
}


bool Int8EntropyCalibrator::getBatch(void* bindings[], const char* names[], int nbBindings)
{
    
    if (m_ImageIndex + m_BatchSize >= m_ImageList.size()) return false;

    std::vector<std::string> batchNames;
    // Load next batch
    for (uint32_t j = m_ImageIndex; j < m_ImageIndex + m_BatchSize; ++j){
        batchNames.push_back(m_ImageList.at(j));
        //dsImages.at(j - m_ImageIndex) = DsImage(m_ImageList.at(j), m_InputH, m_InputW);
    }
    m_ImageIndex += m_BatchSize;
    float *batchData = loadImages(batchNames);

    NV_CUDA_CHECK(cudaMemcpy(m_DeviceInput, batchData, m_InputCount * sizeof(float),
                             cudaMemcpyHostToDevice));
    assert(!strcmp(names[0], m_InputBlobName.c_str()));
    bindings[0] = m_DeviceInput;
    delete[] batchData;
    return true;
}

const void* Int8EntropyCalibrator::readCalibrationCache(size_t& length)
{
    void* output;
    m_CalibrationCache.clear();
    assert(!m_CalibTableFilePath.empty());
    std::ifstream input(m_CalibTableFilePath, std::ios::binary);
    input >> std::noskipws;
    if (m_ReadCache && input.good())
        std::copy(std::istream_iterator<char>(input), std::istream_iterator<char>(),
                  std::back_inserter(m_CalibrationCache));

    length = m_CalibrationCache.size();
    if (length)
    {
        //std::cout << "Using cached calibration table to build the engine" << std::endl;
        output = &m_CalibrationCache[0];
    }

    else
    {
        //std::cout << "New calibration table will be created to build the engine" << std::endl;
        output = nullptr;
    }

    return output;
}

void Int8EntropyCalibrator::writeCalibrationCache(const void* cache, size_t length)
{
    assert(!m_CalibTableFilePath.empty());
    std::ofstream output(m_CalibTableFilePath, std::ios::binary);
    output.write(reinterpret_cast<const char*>(cache), length);
    output.close();
}