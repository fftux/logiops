#include <thread>
#include <sstream>

#include "DeviceMonitor.h"
#include "util.h"

#define NON_WIRELESS_DEV(index) (index) == HIDPP::DefaultDevice ? "default" : "corded"

using namespace logid;
using namespace logid::backend;

/*
void stopAndDeleteConnectedDevice (ConnectedDevice &connected_device)
{
    if(!connected_device.device->waiting_for_receiver)
        log_printf(INFO, "%s (Device %d on %s) disconnected", connected_device.device->name.c_str(),
            connected_device.device->index, connected_device.device->path.c_str());
    connected_device.device->stop();
    connected_device.associatedThread.join();
    delete(connected_device.device);
}

DeviceMonitor::~DeviceMonitor()
{
    this->devices_mutex.lock();
        for (auto it = this->devices.begin(); it != this->devices.end(); it++) {
            for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {
                stopAndDeleteConnectedDevice(jt->second);
            }
        }
    this->devices_mutex.unlock();
}

Device* DeviceMonitor::insertNewDevice(const std::string &path, HIDPP::DeviceIndex index)
{
    auto device = new Device(path, index);
    device->init();

    this->devices_mutex.lock();
        log_printf(INFO, "%s detected: device %d on %s", device->name.c_str(), index, path.c_str());
        auto path_bucket = this->devices.emplace(path, std::map<HIDPP::DeviceIndex, ConnectedDevice>()).first;
        path_bucket->second.emplace(index, ConnectedDevice{
            device,
            std::thread([device]() {
                device->start();
            })
        });
    this->devices_mutex.unlock();

    return device;
}

Device* DeviceMonitor::insertNewReceiverDevice(const std::string &path, HIDPP::DeviceIndex index)
{
    auto *device = new Device(path, index);
    this->devices_mutex.lock();
    auto path_bucket = this->devices.emplace(path, std::map<HIDPP::DeviceIndex, ConnectedDevice>()).first;
    path_bucket->second.emplace(index, ConnectedDevice{
            device,
            std::thread([device]() {
                device->waitForReceiver();
            })
    });
    this->devices_mutex.unlock();

    return device;
}

void DeviceMonitor::stopAndDeleteAllDevicesIn (const std::string &path)
{
    this->devices_mutex.lock();
        auto path_bucket = this->devices.find(path);
        if (path_bucket != this->devices.end())
        {
            for (auto& index_bucket : path_bucket->second) {
                stopAndDeleteConnectedDevice(index_bucket.second);
            }
            this->devices.erase(path_bucket);
        }
    this->devices_mutex.unlock();
}

void DeviceMonitor::stopAndDeleteDevice (const std::string &path, HIDPP::DeviceIndex index)
{
    this->devices_mutex.lock();
        auto path_bucket = this->devices.find(path);
        if (path_bucket != this->devices.end())
        {
            auto index_bucket = path_bucket->second.find(index);
            if (index_bucket != path_bucket->second.end())
            {
                stopAndDeleteConnectedDevice(index_bucket->second);
                path_bucket->second.erase(index_bucket);
            }
        }
    this->devices_mutex.unlock();

    log_printf(WARN, "Attempted to disconnect not previously connected device %d on %s", index, path.c_str());
}
*/

void DeviceMonitor::addDevice(std::string path)
{
    try {
        backend::hidpp::Device device(path, hidpp::DeviceIndex::WirelessDevice1);
        log_printf(DEBUG, "Detected HID++ device at %s", path.c_str());

        backend::hidpp::EventHandler eventHandler;
        eventHandler.condition = [device](backend::hidpp::Report& report)->bool
        {
            return true;
        };
        eventHandler.callback = [device](backend::hidpp::Report& report)->void
        {
            log_printf(DEBUG, "Event on %s:%d", device.devicePath().c_str(),
                    device.deviceIndex());
            for(auto& i : report.rawReport())
                    printf("%02x ", i);
            printf("\n");
        };

        device.addEventHandler("MONITOR_ALL", eventHandler);

        std::thread([](backend::hidpp::Device device) { device.listen(); }, device).detach();

        /* This is a temporary solution to avoid std::bad_function_call */
        while(true) {}
    }
    catch(backend::hidpp::Device::InvalidDevice &e)
    {
        log_printf(DEBUG, "Detected device at %s but %s", path.c_str(), e.what());
    }
    catch(std::system_error &e)
    {
        log_printf(WARN, "Failed to open %s: %s", path.c_str(), e.what());
    }
}

void DeviceMonitor::removeDevice(std::string path)
{
    log_printf(DEBUG, "Device %s disconnected", path.c_str());
    /*
    ipc_server->removeReceiver(path);
    this->stopAndDeleteAllDevicesIn(std::string(path));
     */
}
