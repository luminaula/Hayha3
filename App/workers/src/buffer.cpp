#include "buffer.hpp"
#include "settings.hpp"
#include <atomic>

namespace HBuffer{

    //This is a little weird so rewrite me please. Some of this code is useless
    //Circular buffer of size n framebuffers
    //Capturer handles filling these and does so more often than other workers need them.


    std::vector<Framebuffer> m_framebuffers;

    std::atomic<uint32_t> m_indexCapture;
    std::atomic<uint32_t> m_indexDetect;
    std::atomic<uint32_t> m_indexPresent;

    void init(uint32_t count){
        for(int i=0;i<count;i++){
            m_framebuffers.emplace_back(Framebuffer(Settings::capture.w,Settings::capture.h,Settings::net.w,Settings::net.h,ARGB));
            m_framebuffers.back().resize(Settings::capture.w,Settings::capture.h,Settings::net.w,Settings::net.h,ARGB);
            m_framebuffers.back().uid = i;
        }
        m_indexCapture = 0;
        m_indexDetect = 0;
        m_indexPresent = 0;
    }

    Framebuffer &getFramebuffer(uint32_t index){
        return m_framebuffers.at(index);
    }

    Framebuffer &getFramebuffer(Stage_t stage){
        switch(stage){
            case CAPTURE:
                return m_framebuffers[m_indexCapture];
            case DETECT:
                return m_framebuffers[m_indexDetect];
            case PRESENT:
                return m_framebuffers[m_indexPresent];
            default:
                return m_framebuffers[m_indexCapture];
        }
    }
    void nextFramebuffer(Stage_t stage){
        size_t size = m_framebuffers.size();
        switch(stage){
            case CAPTURE:
                m_indexCapture++;
                if(m_indexCapture == size)
                    m_indexCapture = 0;
                break;
            case DETECT:
                m_indexDetect++;
                if(m_indexDetect == size)
                    m_indexDetect = 0;
                break;
            case PRESENT:
                m_indexPresent++;
                if(m_indexPresent == size)
                    m_indexPresent = 0;
                break;
            default:
                break;
        }
    }

    uint32_t getIndex(Stage_t stage){
        switch(stage){
            case CAPTURE:
                return m_indexCapture.load();
            case DETECT:
                return m_indexDetect.load();
            case PRESENT:
                return m_indexPresent.load();
            default:
                return 0;
        }
    }


    void finishCapture(int id){
        m_indexDetect = id;
    }
    void finishDetect(int id){
        m_indexPresent = id;
    }

    


}