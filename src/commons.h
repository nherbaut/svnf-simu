//
// Created by nherbaut on 15/06/15.
//

#ifndef SVNF_SIMU_COMMONS_H_QSGQSDFQSDFQSDF
#define SVNF_SIMU_COMMONS_H_QSGQSDFQSDFQSDF

#include <sstream>
#include <string>
#include <ns3/ipv4-address.h>

using namespace ns3;

struct RetrieveKey {
    template<typename T>
    typename T::first_type operator()(T keyValuePair) const {
        return keyValuePair.first;
    }
};

class ServerPlotData {
public:
    uint32_t totalSizeTransmitted = 0;


};

class ClientStatObservedFromMain {

public:
    Time startDate;
    Time stopDate;
    uint32_t totalSize = 0;
    uint32_t dataSourceId;


};


class ClientDataFromDataSource {
public:


    std::string getPayloadId() const {
        return m_payloadId;
    }

    uint64_t getTotalTxBytes() const {
        return m_totalSize;
    }

    DataRate getTargetDataRate() const {
        return DataRate(m_targetBitrate);
    }

    ClientDataFromDataSource() {

    }


    ClientDataFromDataSource(std::string data) {
        std::stringstream s(data);
        s >> m_payloadId >> m_totalSize >> m_targetBitrate;

    }

    ClientDataFromDataSource(std::string payloadId, uint64_t totalSize, DataRate targetDataRate) {
        m_payloadId = payloadId;
        m_totalSize = totalSize;
        m_targetBitrate = targetDataRate.GetBitRate();

    }

    std::string toString() {
        std::stringstream s;
        s << m_payloadId << " " << m_totalSize << " " << m_targetBitrate << " " << m_startDate.GetSeconds();
        return s.str();
    }

    std::string getIp() const {
        return m_ip;
    }

    void setIp(std::string m_ip) {
        ClientDataFromDataSource::m_ip = m_ip;
    }

    void setIp(Ipv4Address addr) {
        std::stringstream s;
        s << addr;
        this->setIp(s.str());

    }

    Time getEndDate() const {
        return m_endDate;
    }

    void setEndDate(Time t) {
        ClientDataFromDataSource::m_endDate = t;
    }

    Time getStartDate() const {
        return m_startDate;
    }

    void setStartDate(Time t) {
        ClientDataFromDataSource::m_startDate = t;
    }

    uint64_t getCurrentTxBytes() const {
        return m_currentTxBytes;
    }

    void setCurrentTxBytes(uint64_t currentTxBytes) {
        ClientDataFromDataSource::m_currentTxBytes = currentTxBytes;
    }


    bool isDropped() const {
        return m_dropped;
    }

    void setDropped(bool dropped) {
        ClientDataFromDataSource::m_dropped = dropped;
    }


private:
    std::string m_payloadId;
    uint64_t m_totalSize;
    uint64_t m_targetBitrate;
    Time m_startDate;
    Time m_endDate;
    uint64_t m_currentTxBytes;
    std::string m_ip;
public:
    uint16_t getPort() const {
        return m_port;
    }

    void setPort(uint16_t port) {
        ClientDataFromDataSource::m_port = port;
    }

private:
    uint16_t m_port;

    bool m_dropped = false;
public:
    Time const &getDroppedDate() const {
        return m_droppedDate;
    }

    void setDroppedDate(Time const &droppedDate) {
        ClientDataFromDataSource::m_droppedDate = droppedDate;
    }

private:
    Time m_droppedDate;


};


extern std::map<std::string, ClientDataFromDataSource *> g_clientData;
extern std::map<int, int> newResourcesStartTime;

#endif //SVNF_SIMU_COMMONS_H_QSGQSDFQSDFQSDF
