/******************************************************************************
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
 *****************************************************************************/
#include "SampleICEI.h"
#include <iostream>
#include <vector>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/processRunSet.h>
#include <util/FileUtils.h>
#include <util/DetectorDataArchive.h>
using namespace cvac;


///////////////////////////////////////////////////////////////////////////////
// This is called by IceBox to get the service to communicate with.
extern "C"
{
	//
	// ServiceManager handles all the icebox interactions so we construct
    // it and set a pointer to our detector.
	//
	ICE_DECLSPEC_EXPORT IceBox::Service* create(
                                        Ice::CommunicatorPtr communicator)
	{
        ServiceManager *sMan = new ServiceManager();
        BowICEI *bow = new SampleICEI(sMan);
        sMan->setService(bow, "sample");
        return (::IceBox::Service*) sMan->getIceService();
	}
}


///////////////////////////////////////////////////////////////////////////////

SampleICEI::SampleICEI(ServiceManager *sman)
: mDetector(NULL),mInitialized(false)
{
    mServiceMan = sman;
    mProperties = NULL;
}

SampleICEI::~SampleICEI()
{
}
                          
void SampleICEI::initialize(::Ice::Int verbosity, 
                            const ::DetectorData& data, 
                            const ::Ice::Current& current)
{
	if (mDetector == NULL)
		mDetector = new mDetector();
	
    // Get the default CVAC data directory as defined in the config file
    mCVACDataDir = mServiceMan->getDataDir();	

    // Use utils expandSeq_fromFile to uncompress the
    // detector data and get the file names contained in the archive.
    // Filepath is relative to 'CVAC_DataDir'
    std::string archiveFilePath; 
    archiveFilePath = m_CVAC_DataDir + "/" + 
              data.file.directory.relativePath + "/" + data.file.filename;

    // Use the name of the detector to store the uncompressed data.
    std::string name = getName(current);

    std::vector<std::string> fileNameStrings =  expandSeq_fromFile(
                                          archiveFilePath, name);
		   
    // We will build an absolute path to the uncompressed data.
    // The expandSeq_fromFile function put the data in a subdirectory
    // ".detectorName" within the current working directory.
    std::string dpath;
    std::string directory = getCurrentWorkingDirectory();
    dpath.reserve(directory.length() + name.length() + 3);
    dpath += directory;
    dpath += std::string("/");
    dpath += ".";
    dpath += name;

    // We pass the uncompressed data directory to initialize the
    // detector.
    if(mDetector->detect_initialize(dpath))
        mInitialized = true;
    else
    {
        localAndClientMsg(VLogger::WARN, NULL, "Detector initialize failed\n");
        mInitialized = false;
    }
}

bool SampleICEI::isInitialized(const ::Ice::Current& current)
{
	return fInitialized;
}
 
void SampleICEI::destroy(const ::Ice::Current& current)
{
	if(mDetector != NULL)
		delete mDetector;
	mDetector = NULL;

	mInitialized = false;
}

std::string SampleICEI::getName(const ::Ice::Current& current)
{
	return "sampleDetect";
}
std::string SampleICEI::getDescription(const ::Ice::Current& current)
{
	return "Sample detector - Template for what is required for a CVAC Detector";
}

void SampleICEI::setVerbosity(::Ice::Int verbosity, 
                              const ::Ice::Current& current)
{
   //TODO: need a way for the client to set the verbosity
}


DetectorPropertiesPrx SampleICEI::getDetectorProperties(
                               const ::Ice::Current& current)
{	
    if (mProperties == NULL)
    {
        if (mDetector == NULL)
            return NULL;
        mProperties = new DetectorPropertiesI(mDetector);
        mPropProxy = *((DetectorPropertiesPrx*)mServiceMan->createInterface(
                                          mProperties, current));
    }
    return mPropProxy;

}

ResultSetV2 SampleICEI::processSingleImg(DetectorPtr detector,
                                         const char* fullfilename)
{	
	ResultSetV2 resSet;	
    int bestClass;

	localAndClientMsg(VLogger::DEBUG_1, NULL, "Processing %s.\n",
                                       fullfilename);
	SampleICEI* detect = static_cast<SampleICEI*>(detector.get());
    bool result = detect->getDetector()->detect_run(fullfilename, bestClass);

    if(true == result) {
        localAndClientMsg(VLogger::DEBUG_1, NULL, 
                "Detection, %s as Class: %d\n", fullfilename, bestClass);
    }

    Result tResult;
    tResult.original = new Labelable();
    tResult.original->sub.path.filename = new std::string(fullfilename);

    // The original field is for the original file name.  Results need
    // to be returned in foundLabels.
    Labelable *labelable = new Labelable();
    char buff[32];
    sprintf(buff, "%d", bestClass);
    labelable->lab.name = buff;   
    tResult.foundLabels.push_back(labelable);
    resSet.results.push_back(tResult);
	
	return resSet;
}

void SampleICEI::process(const Ice::Identity &client, const ::RunSet& runset,
                         const ::Ice::Current& current)
{
	DetectorCallbackHandlerPrx callback = 
                     DetectorCallbackHandlerPrx::uncheckedCast(
                       current.con->createProxy(client)->ice_oneway());
    DoDetectFunc func = SampleICEI::processSingleImg;

    try 
    {
        processRunSet(this, callback, func, runset, 
                      mCVACDataDir, mServiceMan);
    }
    catch (exception e) {
        localAndClientMsg(VLogger::ERROR_V, NULL, 
                 "$$$ Detector could not process given file-path.");
    }
}

///////////////////////////////////////////////////////////////////////////////

DetectorPropertiesI::DetectorPropertiesI(SampleDetector *detector)
{
    mDetector = detector;
}

DetectorPropertiesI::~DetectorPropertiesI()
{
}

bool DetectorPropertiesI::isSlidingWindow(const ::Ice::Current &c)
{
    return mDetector->isSlidingWindow();
}

cvac::Size DetectorPropertiesI::getNativeWindowSize(const ::Ice::Current &c)
{
    return mDetector->getNativeWindowSize();
}

void DetectorPropertiesI::getSlidingParameters(cvac::Size &startSize, 
                              cvac::Size &stopSize,
                              float &scaleFactor, float &stepX, float &stepY,
                              const ::Ice::Current &c)
{
    return mDetector->getSlidingParameters(startSize, stopSize,
                              scaleFactor, stepX, stepY);
}

void DetectorPropertiesI::setSlidingParameters(cvac::Size startSize, 
                              cvac::Size stopSize,
                              float scaleFactor, float stepX, float stepY,
                              const ::Ice::Current &c)
{
    mDetector->setSlidingParameters(startSize, stopSize,
                              float scaleFactor, float stepX, float stepY);
}

bool DetectorPropertiesI::canSetSensitivity(const ::Ice::Current &c)
{
    return mDetector->canSetSensitivity();
}

void DetectorPropertiesI::getSensitivity(
                              double &falseAlarmRate, double &recall,
                              const ::Ice::Current &c)
{
    mDetector->getSensitivity(falseAlarmRate, recall);
}

void DetectorPropertiesIsetSensitivity(double falseAlarmRate, double recall,
                              const ::Ice::Current &c)
{
    mDetector->setSensitivity(falseAlarmRate, recall);
}

bool DetectorPropertiesI::canPostProcessNeighbors(const ::Ice::Current &c)
{
    return mDetector->canPostProcessNeighbors();
}

void DetectorPropertiesI::setMinNeighbors(int num, const ::Ice::Current &c)
{
    return mDetector->setMinNeighbors(num);
}
