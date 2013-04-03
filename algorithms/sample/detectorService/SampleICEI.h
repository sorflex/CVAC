/*****************************************************************************
 * CVAC Software Disclaimer
 * 
 * This software was developed at the Naval Postgraduate School, Monterey, CA,
 * by employees of the Federal Government in the course of their official duties.
 * Pursuant to title 17 Section 105 of the United States Code this software
 * is not subject to copyright protection and is in the public domain. It is 
 * an experimental system.  The Naval Postgraduate School assumes no
 * responsibility whatsoever for its use by other parties, and makes
 * no guarantees, expressed or implied, about its quality, reliability, 
 * or any other characteristic.
 * We would appreciate acknowledgement and a brief notification if the software
 * is used.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Naval Postgraduate School, nor the name of
 *       the U.S. Government, nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#ifndef _SampleICEI_H__
#define _SampleICEI_H__

#include <Data.h>
#include <Services.h>
#include <sample.h>

#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <IceUtil/UUID.h>
#include <util/processRunSet.h>
#include <util/ServiceMan.h>


/**
 * Sample CVAC detector instance class to demonstrate the code required to 
 * for a CVAC detector service. This class represents a Detector interface
 * as defined in the Services.ice file.
 */
class SampleICEI : public cvac::Detector
{
public:
    /**
     * The constructor for the Sample Detector service.
     * This will be called once on startup of the service.
     * @param The ServiceManager class that interfaces to ICE.
     */
    SampleICEI(cvac::ServiceManager *servm);

    /**
     * The destructor for the Sample Detector service.  Here clean up
     * any detector specific code. This will be called once when the
     * service is shutdown.
     */
    ~SampleICEI();

    /**
     * This is called by a client to initialize a detector.  It can be assumed
     * that the client will call initialize to initialize a detector and
     * to call destroy when the client is finished with the detector.  It also
     * can be assumed that initialize and destroy will be called multiple times,
     * as much as once per each process call.
     * @param verbosity - the level of logging to be sent to the log file.
     *                    This is an integer from 0 to 6.
     * @param data - The detector data to use.  This is defined in the 
     *               Services.ice file and it usually represents a compressed
     *               zip or tar file containing the detector specific training
     *               data required to run the detector. 
     * @param current - ICE data that contains information about the
     *               interface request being called.
    virtual void initialize(::Ice::Int verbosity,
                            const ::cvac::DetectorData& data,
                            const ::Ice::Current& current);

    /**
     * This is called by a client to see if an initialization call was
     * successfull. 
     * @param current - ICE data that contains information about the
     *               interface request being called.
     * @return - Return true if initialization was successfull else
     *           return false.
     */
    virtual bool isInitialized(const ::Ice::Current& current);

    /**
     * This is called by a client to run the detector on the data defined in
     * the RunSet.
     * @param callback - A CallbackHandler interface (see Services.ice)
     *                   that the service should call to report back to 
     *                   the client results of the detection as they occur.
     * @param runset - The set of data that should be searched by the detector.
     *                 This can be consist of Directories or files.  The
     *                 util library contains a processRunSet function that
     *                 will help in processing the runset.
     * @param current - ICE data that contains information about the
     *               interface request being called.
     */
     
    virtual void process(const Ice::Identity &callback, 
                         const ::cvac::RunSet& runset,
                         const ::Ice::Current& current);

    /**
     * This is called by a client to cleanup a detector.  It can be assumed
     * that the client will call initialize to initialize a detector and
     * to call destroy when the client is finished with the detector.  It also
     * can be assumed that initialize and destroy will be called multiple times,
     * as much as once per each process call.
     * @param current - ICE data that contains information about the
     *               interface request being called.
     */
    virtual void destroy(const ::Ice::Current& current);


    /**
     * This is called by the client to get the name of the detector.  This
     * name is usually presented to the user as a token to represent the 
     * detector and should not contain any spaces.
     * @param current - ICE data that contains information about the
     *               interface request being called.
    virtual std::string getName(const ::Ice::Current& current);

    /**
     * This is called by the client to get the description of the detector.  
     * This should be a short one sentence description of what this detector
     * detects.
     * @param current - ICE data that contains information about the
     *               interface request being called.
    virtual std::string getDescription(const ::Ice::Current& current);

    /**
     * This is called by the client to control the logging level of 
     * messages output by the service.
     * @param verbosity - The level of logging to be sent to the log file.
     *                    This is an integer from 0 to 6.
     * @param data - The detector data to use.  This is defined in the 
     *               Services.ice file and it usually represents a compressed
     *               zip or tar file containing the detector specific training
     *               data required to run the detector. 
     * @param current - ICE data that contains information about the
     *               interface request being called.
    void setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current);

    /**
     * This is called by the client to get an interface for the 
     * DetectorProperties as defined in Services.ice.  This properties
     * interface allows the client to make adjustments to how the
     * detector behaves.
     * @param current - ICE data that contains information about the
     *               interface request being called.
     * @return - A proxy to a DetectorProperties interface.
     */
    virtual cvac::DetectorPropertiesPrx getDetectorProperties(
                                             const ::Ice::Current& current);

private:

    /**
     * A pointer to the cvac utils ServiceManager class that manages
     * the ICE Service interface.
     */
    cvac::ServiceManager   *mServiceMan;

    /**
     * A pointer to the actual detector class.
     */
    SampleDetector         *mDetector;
    
    /**
     * A flag to indicate if initialization was successfull
     */
    bool                    mInitialized;    

    /**
     * The data directory defined in the ice configuration to where
     * the data is.
     */
    std::string             mCVACDataDir; 

    /**
     * The detector properties
     */
    DetectorPropertiesI     *mProperties;
    DetectorPropertiesPrx   mPropProxy;

    /**
     * This is the callback function passed in the cvac util function
     * processRunSet.  It will be called for each file in the RunSet.
     * @param detector - This is a pointer to this class.
     * @param fullfilename - The image to run the detector on.
     * @return Return the results of the detection.  See Data.ice for 
     *         the definition of a cvac::ResultSetV2 result.
    static cvac::ResultSetV2 processSingleImg(
                           cvac::DetectorPtr detector,
                           const char* fullfilename);
};

/**
 * Interface class for the client to get at a detectors properties
 */
class DetectorPropertiesI : public cvac::DetectorProperties
{
public:
    DetectorPropertiesI(SampleDetector *detector);
    ~DetectorPropertiesI();
    bool isSlidingWindow(const ::Ice::Current &c);
    cvac::Size getNativeWindowSize(const ::Ice::Current &c);
    void getSlidingParameters(cvac::Size &startSize, cvac::Size &stopSize;
                              float &scaleFactor, float &stepX, float &stepY,
                              const ::Ice::Current &c);
    void setSlidingParameters(cvac::Size startSize, cvac::Size stopSize;
                              float scaleFactor, float stepX, float stepY,
                              const ::Ice::Current &c);
    bool canSetSensitivity(const ::Ice::Current &c);
    void getSensitivity(double &falseAlarmRate, double &recall,
                              const ::Ice::Current &c);
    void setSensitivity(double falseAlarmRate, double recall,
                              const ::Ice::Current &c);
    bool canPostProcessNeighbors(const ::Ice::Current &c);
    void setMinNeighbors(int num, const ::Ice::Current &c);
private:
    SampleDetector *mdetector;
};

#endif //_SampleICEI_H__
