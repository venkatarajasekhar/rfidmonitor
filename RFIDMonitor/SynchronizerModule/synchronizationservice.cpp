/****************************************************************************
**
** WWW.FISHMONITORING.COM.BR
**
** Copyright (C) 2013
**                     Luis Valdes <luisvaldes88@gmail.com>
**
** This file is part of the FishMonitoring project
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; version 2
** of the License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
****************************************************************************/

#include <QtConcurrent>
#include <QJsonDocument>
#include <QJsonObject>

#include <future>
#include <functional>

#include <rfidmonitor.h>
#include <logger.h>

#include <json/nodejsmessage.h>
#include <json/synchronizationpacket.h>

#include "synchronizationservice.h"

SynchronizationService::SynchronizationService(QObject *parent) :
    SynchronizationInterface(parent)
{
}

void SynchronizationService::readyRead()
{
    static PackagerInterface *packager = 0;
    static CommunicationInterface *communitacion = 0;
    if(!packager || !communitacion) {
        packager = qobject_cast<PackagerInterface *>(RFIDMonitor::instance()->defaultService(ServiceType::KPackager));
        communitacion = qobject_cast<CommunicationInterface *>(RFIDMonitor::instance()->defaultService(ServiceType::KCommunicator));
    }
    if(packager /*&& !m_timer.remainingTime()*/) {

        packager->generatePackets();

        if(RFIDMonitor::instance()->isconnected()){
            QMap<QString, QByteArray> allData = packager->getAll();

            if(communitacion) {
                QMap<QString, QByteArray>::iterator i;
                Logger::instance()->writeRecord(Logger::severity_level::debug, "synchronizer", Q_FUNC_INFO, QString("Sending %1 Packets to server").arg(allData.size()));
                for(i = allData.begin(); i != allData.end(); ++i){

                    json::NodeJSMessage answer;
                    answer.setType("DATA");
                    answer.setDateTime(QDateTime::currentDateTime());
                    answer.setJsonData(QJsonDocument::fromJson(QString(i.value()).toLatin1()).object());
                    QJsonObject jsonAnswer;
                    answer.write(jsonAnswer);

//#ifdef CPP_11_ASYNC
//                    /*C++11 std::async Version*/
//                    std::function<void (QByteArray)> sendMessage = std::bind(&CommunicationInterface::sendMessage, communitacion, std::placeholders::_1);
//                    std::async(std::launch::async, sendMessage, QJsonDocument(jsonAnswer).toJson());
//#else
//                    /*Qt Concurrent Version*/
//                    QtConcurrent::run(communitacion, &CommunicationInterface::sendMessage, QJsonDocument(jsonAnswer).toJson());
//#endif
                    communitacion->sendMessage(QJsonDocument(jsonAnswer).toJson());
                }
            }else{
                Logger::instance()->writeRecord(Logger::severity_level::debug, "synchronizer", Q_FUNC_INFO, QString("Packager is not working!"));
            }
        }
    }
}

QString SynchronizationService::serviceName() const
{
    return "synchronization.service";
}

void SynchronizationService::init()
{

}

ServiceType SynchronizationService::type()
{
    return ServiceType::KSynchronizer;
}

void SynchronizationService::onDataReceived(QString data)
{
    (void)data;
}

void SynchronizationService::sendData()
{

}
