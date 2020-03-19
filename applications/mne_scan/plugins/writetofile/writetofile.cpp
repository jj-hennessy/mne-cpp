//=============================================================================================================
/**
 * @file     writetofile.cpp
 * @author   Lorenz Esch <lesch@mgh.harvard.edu>
 * @version  dev
 * @date     February, 2020
 *
 * @section  LICENSE
 *
 * Copyright (C) 2020, Lorenz Esch. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @brief    Definition of the WriteToFile class.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "writetofile.h"

#include "FormFiles/writetofilesetupwidget.h"

#include <disp/viewers/projectsettingsview.h>
#include <scMeas/realtimemultisamplearray.h>
#include <fiff/fiff_stream.h>

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace WRITETOFILEPLUGIN;
using namespace SCMEASLIB;
using namespace IOBUFFER;
using namespace DISPLIB;
using namespace FIFFLIB;
using namespace SCSHAREDLIB;
using namespace Eigen;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

WriteToFile::WriteToFile()
: m_iBlinkStatus(0)
, m_bWriteToFile(false)
, m_bUseRecordTimer(false)
, m_iRecordingMSeconds(5*60*1000)
, m_iSplitCount(0)
, m_pCircularBuffer(CircularBuffer_Matrix_double::SPtr(new CircularBuffer_Matrix_double(40)))
{
    m_pActionRecordFile = new QAction(QIcon(":/images/record.png"), tr("Start Recording"),this);
    m_pActionRecordFile->setStatusTip(tr("Start Recording"));
    connect(m_pActionRecordFile.data(), &QAction::triggered,
            this, &WriteToFile::toggleRecordingFile);
    addPluginAction(m_pActionRecordFile);

    //Init timers
    if(!m_pRecordTimer) {
        m_pRecordTimer = QSharedPointer<QTimer>(new QTimer(this));
        m_pRecordTimer->setSingleShot(true);
        connect(m_pRecordTimer.data(), &QTimer::timeout,
                this, &WriteToFile::toggleRecordingFile);
    }

    if(!m_pBlinkingRecordButtonTimer) {
        m_pBlinkingRecordButtonTimer = QSharedPointer<QTimer>(new QTimer(this));
        connect(m_pBlinkingRecordButtonTimer.data(), &QTimer::timeout,
                this, &WriteToFile::changeRecordingButton);
    }
}

//=============================================================================================================

WriteToFile::~WriteToFile()
{
    if(this->isRunning()) {
        stop();
    }
}

//=============================================================================================================

QSharedPointer<IPlugin> WriteToFile::clone() const
{
    QSharedPointer<WriteToFile> pWriteToFileClone(new WriteToFile);
    return pWriteToFileClone;
}

//=============================================================================================================

void WriteToFile::init()
{
    // Input
    m_pWriteToFileInput = PluginInputData<RealTimeMultiSampleArray>::create(this, "WriteToFileIn", "WriteToFile input data");
    connect(m_pWriteToFileInput.data(), &PluginInputConnector::notify,
            this, &WriteToFile::update, Qt::DirectConnection);
    m_inputConnectors.append(m_pWriteToFileInput);
}

//=============================================================================================================

void WriteToFile::unload()
{
}

//=============================================================================================================

bool WriteToFile::start()
{
    QThread::start();

    return true;
}

//=============================================================================================================

bool WriteToFile::stop()
{
    requestInterruption();
    wait();

    return true;
}

//=============================================================================================================

IPlugin::PluginType WriteToFile::getType() const
{
    return _IAlgorithm;
}

//=============================================================================================================

QString WriteToFile::getName() const
{
    return "Write To File";
}

//=============================================================================================================

QWidget* WriteToFile::setupWidget()
{
    WriteToFileSetupWidget* setupWidget = new WriteToFileSetupWidget(this);//widget is later distroyed by CentralWidget - so it has to be created everytime new
    return setupWidget;
}

//=============================================================================================================

void WriteToFile::update(SCMEASLIB::Measurement::SPtr pMeasurement)
{
    if(QSharedPointer<RealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<RealTimeMultiSampleArray>()) {
        //Check if the fiff info was inititalized
        if(!m_pFiffInfo) {
            m_pFiffInfo = pRTMSA->info();
            initPluginControlWidgets();
        }

        // Check if data is present
        if(pRTMSA->getMultiSampleArray().size() > 0) {
            for(unsigned char i = 0; i < pRTMSA->getMultiSampleArray().size(); ++i) {
                // Please note that we do not need a copy here since this function will block until
                // the buffer accepts new data again. Hence, the data is not deleted in the actual
                // Mesaurement function after it emitted the notify signal.
                while(!m_pCircularBuffer->push(pRTMSA->getMultiSampleArray()[i])) {
                    //Do nothing until the circular buffer is ready to accept new data again
                }
            }
        }
    }
}

//=============================================================================================================

void WriteToFile::initPluginControlWidgets()
{
    if(m_pFiffInfo) {
        QList<QWidget*> plControlWidgets;

        //Mne Scan data Path
        QString sMneScanDataPath = QDir::homePath() + "/mne_scan";
        if(!QDir(sMneScanDataPath).exists()) {
            QDir().mkdir(sMneScanDataPath);
        }

        //Test Project
        QSettings settings;
        QString sCurrentProject = settings.value(QString("MNESCAN/%1/currentProject").arg(getName()), "TestProject").toString();
        if(!QDir(sMneScanDataPath+"/"+sCurrentProject).exists()) {
            QDir().mkdir(sMneScanDataPath+"/"+sCurrentProject);
        }

        //Test Subject
        QString sCurrentSubject = settings.value(QString("MNESCAN/%1/currentSubject").arg(getName()), "TestSubject").toString();
        if(!QDir(sMneScanDataPath+"/"+sCurrentProject+"/"+sCurrentSubject).exists()) {
            QDir().mkdir(sMneScanDataPath+"/"+sCurrentProject+"/"+sCurrentSubject);
        }

        // Projects Settings
        ProjectSettingsView* pProjectSettingsView = new ProjectSettingsView(sMneScanDataPath,
                                                                            sCurrentProject,
                                                                            sCurrentSubject,
                                                                            "");
        pProjectSettingsView->setObjectName("widget_");
        m_sRecordFileName = pProjectSettingsView->getCurrentFileName();

        connect(pProjectSettingsView, &ProjectSettingsView::timerChanged,
                this, &WriteToFile::setRecordingTimerChanged);

        connect(pProjectSettingsView, &ProjectSettingsView::recordingTimerStateChanged,
                this, &WriteToFile::setRecordingTimerStateChanged);

        connect(pProjectSettingsView, &ProjectSettingsView::fileNameChanged,
                this, &WriteToFile::onFileNameChanged);

        connect(pProjectSettingsView, &ProjectSettingsView::fileNameChanged, [=]() {
            pProjectSettingsView->setRecordingElapsedTime(m_recordingStartedTime.elapsed());
        });

        plControlWidgets.append(pProjectSettingsView);

        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

        if(!m_pUpdateTimeInfoTimer) {
            m_pUpdateTimeInfoTimer = QSharedPointer<QTimer>(new QTimer(this));
            connect(m_pUpdateTimeInfoTimer.data(), &QTimer::timeout, [=]() {
                pProjectSettingsView->setRecordingElapsedTime(m_recordingStartedTime.elapsed());
            });
        }
    }
}

//=============================================================================================================

void WriteToFile::run()
{
    MatrixXd matData;
    qint32 size = 0;

    while(!isInterruptionRequested()) {
        if(m_pCircularBuffer) {
            //pop matrix
            if(m_pCircularBuffer->pop(matData)) {
                //Write raw data to fif file
                m_mutex.lock();
                if(m_bWriteToFile) {
                    size += matData.rows()*matData.cols() * 4;

                    if(size > MAX_DATA_LEN) {
                        size = 0;
                        this->splitRecordingFile();
                    }

                    m_pOutfid->write_raw_buffer(matData.cast<double>());
                } else {
                    size = 0;
                }
                m_mutex.unlock();
            }
        }
    }

    //Close the fif output stream
    if(m_bWriteToFile) {
        this->toggleRecordingFile();
    }
}

//=============================================================================================================

void WriteToFile::setRecordingTimerChanged(int timeMSecs)
{
    //If the recording time is changed during the recording, change the timer
    if(m_bWriteToFile)
        m_pRecordTimer->setInterval(timeMSecs-m_recordingStartedTime.elapsed());

    m_iRecordingMSeconds = timeMSecs;
}

//=============================================================================================================

void WriteToFile::setRecordingTimerStateChanged(bool state)
{
    m_bUseRecordTimer = state;
}

//=============================================================================================================

void WriteToFile::onFileNameChanged(const QString& sFileName)
{
    m_sRecordFileName = sFileName;
}

//=============================================================================================================

void WriteToFile::toggleRecordingFile()
{
    //Setup writing to file
    if(m_bWriteToFile) {
        m_mutex.lock();
        m_pOutfid->finish_writing_raw();
        m_mutex.unlock();

        m_bWriteToFile = false;
        m_iSplitCount = 0;

        //Stop record timer
        m_pRecordTimer->stop();
        m_pUpdateTimeInfoTimer->stop();
        m_pBlinkingRecordButtonTimer->stop();

        m_pActionRecordFile->setIcon(QIcon(":/images/record.png"));
    } else {
        m_iSplitCount = 0;

        if(!m_pFiffInfo) {
            QMessageBox msgBox;
            msgBox.setText("FiffInfo missing!");
            msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
            msgBox.exec();
            return;
        }

        if(m_pFiffInfo->dev_head_t.trans.isIdentity()) {
            QMessageBox msgBox;
            msgBox.setText("It seems that no HPI fitting was performed. This is your last chance!");
            msgBox.setInformativeText("Do you want to continue without HPI fitting?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
            int ret = msgBox.exec();
            if(ret == QMessageBox::No)
                return;
        }

        //Initiate the stream for writing to the fif file
        m_qFileOut.setFileName(m_sRecordFileName);
        if(m_qFileOut.exists()) {
            QMessageBox msgBox;
            msgBox.setText("The file you want to write already exists.");
            msgBox.setInformativeText("Do you want to overwrite this file?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
            int ret = msgBox.exec();
            if(ret == QMessageBox::No) {
                return;
            }
        }

        //Set all projectors to zero before writing to file because we always write the raw data
        for(int i = 0; i<m_pFiffInfo->projs.size(); i++) {
            m_pFiffInfo->projs[i].active = false;
        }

        //Start/Prepare writing process. Actual writing is done in run() method.
        m_mutex.lock();
        RowVectorXd cals;
        m_pOutfid = FiffStream::start_writing_raw(m_qFileOut,
                                                  *m_pFiffInfo,
                                                  cals);
        fiff_int_t first = 0;
        m_pOutfid->write_int(FIFF_FIRST_SAMPLE, &first);
        m_mutex.unlock();

        m_bWriteToFile = true;

        //Start timers for record button blinking, recording timer and updating the elapsed time in the proj widget
        m_pBlinkingRecordButtonTimer->start(500);
        m_recordingStartedTime.restart();
        m_pUpdateTimeInfoTimer->start(1000);

        if(m_bUseRecordTimer) {
            m_pRecordTimer->start(m_iRecordingMSeconds);
        }
    }
}

//=============================================================================================================

void WriteToFile::splitRecordingFile()
{
    //qDebug() << "Split recording file";
    ++m_iSplitCount;
    QString nextFileName = m_sRecordFileName.remove("_raw.fif");
    nextFileName += QString("-%1_raw.fif").arg(m_iSplitCount);

    //Write the link to the next file
    qint32 data;
    m_pOutfid->start_block(FIFFB_REF);
    data = FIFFV_ROLE_NEXT_FILE;
    m_pOutfid->write_int(FIFF_REF_ROLE,&data);
    m_pOutfid->write_string(FIFF_REF_FILE_NAME, nextFileName);
    m_pOutfid->write_id(FIFF_REF_FILE_ID);//ToDo meas_id
    data = m_iSplitCount - 1;
    m_pOutfid->write_int(FIFF_REF_FILE_NUM, &data);
    m_pOutfid->end_block(FIFFB_REF);

    //finish file
    m_pOutfid->finish_writing_raw();

    //start next file
    m_qFileOut.setFileName(nextFileName);
    RowVectorXd cals;
    MatrixXi sel;
    m_pOutfid = FiffStream::start_writing_raw(m_qFileOut,
                                              *m_pFiffInfo,
                                              cals,
                                              sel,
                                              false);
    fiff_int_t first = 0;
    m_pOutfid->write_int(FIFF_FIRST_SAMPLE, &first);
}

//=============================================================================================================

void WriteToFile::changeRecordingButton()
{
    if(m_iBlinkStatus == 0) {
        m_pActionRecordFile->setIcon(QIcon(":/images/record.png"));
        m_iBlinkStatus = 1;
    } else {
        m_pActionRecordFile->setIcon(QIcon(":/images/record_active.png"));
        m_iBlinkStatus = 0;
    }
}