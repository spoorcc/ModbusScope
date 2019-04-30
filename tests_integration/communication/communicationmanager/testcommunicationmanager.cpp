
#include <QtTest/QtTest>
#include <QMap>

#include "modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testcommunicationmanager.h"

void TestCommunicationManager::init()
{
    _pSettingsModel = new SettingsModel;
    _pGuiModel = new GuiModel;
    _pErrorLogModel = new ErrorLogModel;

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_0, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_0, 1);

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_1, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_1, 5021);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_1, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_1, 2);

    _pSettingsModel->setPollTime(100);

    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _serverConnectionDataList.append(QUrl());
        _serverConnectionDataList.last().setPort(_pSettingsModel->port(idx));
        _serverConnectionDataList.last().setHost(_pSettingsModel->ipAddress(idx));

        _testSlaveDataList.append(new TestSlaveData());
        _testSlaveModbusList.append(new TestSlaveModbus(_testSlaveDataList.last()));

        QVERIFY(_testSlaveModbusList.last()->connect(_serverConnectionDataList.last(), _pSettingsModel->slaveId(idx)));
    }
}

void TestCommunicationManager::cleanup()
{
    delete _pSettingsModel;
    delete _pGuiModel;
    delete _pErrorLogModel;

    for (int idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    qDeleteAll(_testSlaveDataList);
    qDeleteAll(_testSlaveModbusList);

    _serverConnectionDataList.clear();
    _testSlaveDataList.clear();
    _testSlaveModbusList.clear();
}

void TestCommunicationManager::singleSlaveSuccess()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(0, 5);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_0]->setRegisterValue(1, 65000);

    GraphDataModel graphDataModel;
    graphDataModel.add();
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setRegisterAddress(1, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(10));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({true, true});
    QList<double> valueList({5, 65000});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}

void TestCommunicationManager::singleSlaveFail()
{
    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    GraphDataModel graphDataModel;
    graphDataModel.add();
    graphDataModel.setRegisterAddress(0, 40001);

    graphDataModel.add();
    graphDataModel.setRegisterAddress(1, 40002);

    CommunicationManager conMan(_pSettingsModel, _pGuiModel, &graphDataModel, _pErrorLogModel);

    QSignalSpy spyReceivedData(&conMan, &CommunicationManager::handleReceivedData);

    /*-- Start communication --*/
    QVERIFY(conMan.startCommunication());

    QVERIFY(spyReceivedData.wait(150));
    QCOMPARE(spyReceivedData.count(), 1);

    QList<QVariant> arguments = spyReceivedData.takeFirst(); // take the first signal

    QList<bool> resultList({false, false});
    QList<double> valueList({0, 0});

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, resultList, valueList);
}


void TestCommunicationManager::verifyReceivedDataSignal(QList<QVariant> arguments, QList<bool> expResultList, QList<double> expValueList)
{
    /* Verify result */
    QVERIFY((arguments[0].canConvert<QList<bool> >()));
    QList<bool> resultList = arguments[0].value<QList<bool> >();
    QCOMPARE(resultList.count(), expResultList.size());

    for(int idx = 0; idx < resultList.size(); idx++)
    {
        QCOMPARE(resultList[idx], expResultList[idx]);
    }

    /* Verify values */
    QVERIFY((arguments[1].canConvert<QList<double> >()));
    QList<double> valueList = arguments[1].value<QList<double> >();
    QCOMPARE(valueList.count(), expValueList.size());

    for(int idx = 0; idx < resultList.size(); idx++)
    {
        QCOMPARE(valueList[idx], expValueList[idx]);
    }
}

QTEST_GUILESS_MAIN(TestCommunicationManager)
