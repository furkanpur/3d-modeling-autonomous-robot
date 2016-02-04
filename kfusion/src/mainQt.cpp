/*

 Copyright (c) 2014 University of Edinburgh, Imperial College, University of Manchester.
 Developed in the PAMELA project, EPSRC Programme Grant EP/K008730/1

 This code is licensed under the MIT License.

 */
#include <kernels.h>
#include <interface.h>
#include <stdint.h>
#include <tick.h>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include <perfstats.h>
#include <PowerMonitor.h>

#include "socket.h"
#include "parameters.h"

#define ADDRESS "10.42.0.10"
#define PORT 4000

char buffer[256];
int sockfd;

static cv::Point prevPoint;

static int x_a = 511;
static int y_a = 400;

#ifndef __QT__

#include <draw.h>
#endif

PerfStats Stats;
PowerMonitor *powerMonitor = NULL;
uint16_t * inputDepth = NULL;
static uchar3 * inputRGB = NULL;
static uchar4 * depthRender = NULL;
static uchar4 * trackRender = NULL;
static uchar4 * volumeRender = NULL;
static DepthReader *reader = NULL;
static Kfusion *kfusion = NULL;
/*
 int          compute_size_ratio = default_compute_size_ratio;
 std::string  input_file         = "";
 std::string  log_file           = "" ;
 std::string  dump_volume_file   = "" ;
 float3       init_poseFactors   = default_initial_pos_factor;
 int          integration_rate   = default_integration_rate;
 float3       volume_size        = default_volume_size;
 uint3        volume_resolution  = default_volume_resolution;
 */
DepthReader *createReader(Configuration *config, std::string filename = "");
int processAll(DepthReader *reader, bool processFrame, bool renderImages,
               Configuration *config, bool reset = false);

inline double tock()
{
    synchroniseDevices();
#ifdef __APPLE__
    clock_serv_t cclock;
    mach_timespec_t clockData;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &clockData);
    mach_port_deallocate(mach_task_self(), cclock);
#else
    struct timespec clockData;
    clock_gettime(CLOCK_MONOTONIC, &clockData);
#endif
    return (double) clockData.tv_sec + clockData.tv_nsec / 1000000000.0;
}

void qtLinkKinectQt(int argc, char *argv[], Kfusion **_kfusion,
                    DepthReader **_depthReader, Configuration *config, void *depthRender,
                    void *trackRender, void *volumeModel, void *inputRGB);
void storeStats(int frame, double *timings, float3 pos, bool tracked,
                bool integrated)
{
    Stats.sample("frame", frame, PerfStats::FRAME);
    Stats.sample("acquisition", timings[1] - timings[0], PerfStats::TIME);
    Stats.sample("preprocessing", timings[2] - timings[1], PerfStats::TIME);
    Stats.sample("tracking", timings[3] - timings[2], PerfStats::TIME);
    Stats.sample("integration", timings[4] - timings[3], PerfStats::TIME);
    Stats.sample("raycasting", timings[5] - timings[4], PerfStats::TIME);
    Stats.sample("rendering", timings[6] - timings[5], PerfStats::TIME);
    Stats.sample("computation", timings[5] - timings[1], PerfStats::TIME);
    Stats.sample("total", timings[6] - timings[0], PerfStats::TIME);
    Stats.sample("X", pos.x, PerfStats::DISTANCE);
    Stats.sample("Y", pos.y, PerfStats::DISTANCE);
    Stats.sample("Z", pos.z, PerfStats::DISTANCE);
    Stats.sample("tracked", tracked, PerfStats::INT);
    Stats.sample("integrated", integrated, PerfStats::INT);
}

/***
 * This program loop over a scene recording
 */

int main(int argc, char ** argv)
{

    prevPoint.x = 511;
    prevPoint.y = 400;

    Configuration config(argc, argv);
    powerMonitor = new PowerMonitor();

    // ========= READER INITIALIZATION  =========
    reader = createReader(&config);

    //  =========  BASIC PARAMETERS  (input size / computation size )  =========
    uint2 inputSize =
        (reader != NULL) ? reader->getinputSize() : make_uint2(640, 480);
    const uint2 computationSize = make_uint2(
                                      inputSize.x / config.compute_size_ratio,
                                      inputSize.y / config.compute_size_ratio);

    // ========= SOCKET ============

    try
    {
        int portno;
        struct sockaddr_in serv_addr;
        struct hostent *server;

        portno = PORT;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            printf("Error opening socket\n");
        server = gethostbyname(ADDRESS);
        if (server == NULL)
        {
            printf("Error: no such host\n");
            exit(0);
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            printf("Error connecting\n");
    }
    catch(std::exception& e)
    {
        std::cout << "sockette hata olustu !" << std::endl;
    }
    // ========= SOCKET END ========

    //  =========  BASIC BUFFERS  (input / output )  =========

    // Construction Scene reader and input buffer
    int width = 640;
    int height = 480;
    //we could allocate a more appropriate amount of memory (less) but this makes life hard if we switch up resolution later;
    inputDepth = (uint16_t*) malloc(sizeof(uint16_t) * width * height);
    inputRGB = (uchar3*) malloc(sizeof(uchar3) * inputSize.x * inputSize.y);
    depthRender = (uchar4*) malloc(sizeof(uchar4) * width * height);
    trackRender = (uchar4*) malloc(sizeof(uchar4) * width * height);
    volumeRender = (uchar4*) malloc(sizeof(uchar4) * width * height);

    float3 init_pose = config.initial_pos_factor * config.volume_size;
    kfusion = new Kfusion(computationSize, config.volume_resolution,
                          config.volume_size, init_pose, config.pyramid);

    //temporary fix to test rendering fullsize
    config.render_volume_fullsize = false;

    //The following runs the process loop for processing all the frames, if QT is specified use that, else use GLUT
    //We can opt to not run the gui which would be faster
    if (!config.no_gui)
    {
#ifdef __QT__
        qtLinkKinectQt(argc,argv, &kfusion, &reader, &config, depthRender, trackRender, volumeRender, inputRGB);
#else
        if ((reader == NULL) || (reader->cameraActive == false))
        {
            std::cerr << "No valid input file specified\n";
            exit(1);
        }
        while (processAll(reader, true, true, &config, false) == 0)
        {
            drawthem(inputRGB, depthRender, trackRender, volumeRender,
                     trackRender, kfusion->getComputationResolution());
        }
#endif
    }
    else
    {
        if ((reader == NULL) || (reader->cameraActive == false))
        {
            std::cerr << "No valid input file specified\n";
            exit(1);
        }
        while (processAll(reader, true, true, &config, false) == 0)
        {
        }
    }
    // ==========     DUMP VOLUME      =========

    if (config.dump_volume_file != "")
    {
        kfusion->dumpVolume(config.dump_volume_file);
    }

    if (config.log_file != "")
    {
        std::ofstream logStream(config.log_file.c_str());
        Stats.print_all_data(logStream);
        logStream.close();
    }

    if (powerMonitor && powerMonitor->isActive())
    {
        std::ofstream powerStream("power.rpt");
        powerMonitor->powerStats.print_all_data(powerStream);
        powerStream.close();
    }

    //  =========  FREE BASIC BUFFERS  =========

    free(inputDepth);
    free(depthRender);
    free(trackRender);
    free(volumeRender);

}

int processAll(DepthReader *reader, bool processFrame, bool renderImages,
               Configuration *config, bool reset)
{
    static float duration = tick();
    static int frameOffset = 0;
    static bool firstFrame = true;
    bool tracked, integrated, raycasted;
    double start, end, startCompute, endCompute;
    uint2 render_vol_size;
    double timings[7];
    float3 pos;
    int frame;
    const uint2 inputSize =
        (reader != NULL) ? reader->getinputSize() : make_uint2(640, 480);
    float4 camera =
        (reader != NULL) ?
        (reader->getK() / config->compute_size_ratio) :
        make_float4(0.0);
    if (config->camera_overrided)
        camera = config->camera / config->compute_size_ratio;

    if (reset)
    {
        frameOffset = reader->getFrameNumber();
    }
    bool finished = false;

    if (processFrame)
    {
        Stats.start();
    }
    timings[0] = tock();
    if (processFrame && (reader->readNextDepthFrame(inputRGB, inputDepth)))
    {
        frame = reader->getFrameNumber() - frameOffset;
        if (powerMonitor != NULL && !firstFrame)
            powerMonitor->start();

        pos = kfusion->getPosition();

        timings[1] = tock();

        kfusion->preprocessing(inputDepth, inputSize);

        timings[2] = tock();

        tracked = kfusion->tracking(camera, config->icp_threshold,
                                    config->tracking_rate, frame);

        timings[3] = tock();

        integrated = kfusion->integration(camera, config->integration_rate,
                                          config->mu, frame);

        timings[4] = tock();

        raycasted = kfusion->raycasting(camera, config->mu, frame);

        timings[5] = tock();

        // ================= OPENCV ===================

        double minVal;
        double maxVal;
        cv::Point minLoc;
        cv::Point maxLoc;

        cv::Mat mScaledDepth = reader->getDepthMapCV();

        cv::Mat bw;
        cv::threshold(mScaledDepth,bw,128,255,0);

        cv::Mat labelImage(mScaledDepth.size(), CV_32S);
        cv::Mat stats, centroids;

        int nLabels = connectedComponentsWithStats(bw, labelImage, stats, centroids, 8);

        int avg_x = 0, avg_y = 0, avg_val = 0;
        int counter = 0, sum_area = 0;

        int left_dens, right_dens;

        //std::cout << "---------------------" << std::endl;
        for(int i = 0 ; i < nLabels ; i++)
        {
            cv::Point p;
            p.x = centroids.at<double>(i,0);
            p.y = centroids.at<double>(i,1);

            if(stats.at<int>(i,4) > 1500)
            {
                //std::cout << p.x << "-" << p.y << " --> " << stats.at<int>(i,4) <<  " --> "<<(int)mScaledDepth.at<uchar>(p.x,p.y) << std::endl;

                if(p.x > 320)
                    right_dens++;
                else
                    left_dens++;

                counter++;


                avg_x += p.x * stats.at<int>(i,4);
                avg_y += p.y * stats.at<int>(i,4);

                sum_area += stats.at<int>(i,4);

                avg_val += (int)mScaledDepth.at<uchar>(p.x,p.y);
            }
        }

        //std::cout << "---------------------" << std::endl;
        cv::Point p;

        p.x = avg_x / sum_area;
        p.y = avg_y / sum_area;

        avg_val /= counter;

        p.x = p.x * 1023 / 639;

        if(avg_val < WALL_THRES)
        {

            if(left_dens > right_dens)
            {
                p.x = 621;
            }
            else
            {
                p.x = 401;
            }

            p.y = 511;

        }
        else if(avg_val < TURN_THRES )
        {
            left_dens = 0; right_dens = 0;
            p.x = ALPHA * prevPoint.x + (1 - ALPHA) * p.x;
            p.y = ALPHA * prevPoint.y + (1 - ALPHA) * p.y;
            p.y = 400;

        }
        else
        {
            left_dens = 0; right_dens = 0;
            p.x = prevPoint.x;
            p.y = prevPoint.y;
            p.y = 400;

        }


        // ========== SOCKET =============
        std::stringstream ss;

        ss << '\u000b' << "jy://" << p.x << "," << p.y << ",0" << '\u001c' << '\r';
        std::string data = ss.str();
        client_send_point(buffer,sockfd,data);
        // ========= SOCKET END ==========

        prevPoint.x = p.x;
        prevPoint.y = p.y;

        //====================== OPENCV END =====================
    }
    else
    {
        if (processFrame)
        {
            finished = true;
            timings[0] = tock();
        }

    }
    if (renderImages)
    {
        kfusion->renderDepth(depthRender, kfusion->getComputationResolution());
        kfusion->renderTrack(trackRender, kfusion->getComputationResolution());
        kfusion->renderVolume(volumeRender, kfusion->getComputationResolution(),
                              (processFrame ? reader->getFrameNumber() - frameOffset : 0),
                              config->rendering_rate, camera, 0.75 * config->mu);
        timings[6] = tock();
    }

    if (processFrame)
    {
        if (powerMonitor != NULL && !firstFrame)
            powerMonitor->sample();
        storeStats(frame, timings, pos, tracked, integrated);

        if (config->no_gui && (config->log_file == ""))
            Stats.print();
        firstFrame = false;
    }
    return (finished);
}

