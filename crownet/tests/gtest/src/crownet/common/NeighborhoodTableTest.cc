#include "crownet/neighbourhood/NeighborhoodTable.h"

#include "main_test.h"
#include "gmock/gmock.h"

using namespace crownet;
using ::testing::Return;

class MockNeighborhoodTable : public NeighborhoodTable{

  public:
    MOCK_METHOD2(scheduleAt, void(simtime_t t, cMessage* msg));
    MOCK_METHOD0(checkAllTimeToLive, void());
    MockNeighborhoodTable(){}

};

TEST(NeighborhoodTableBase, Test_constructor) {
  NeighborhoodTable nTable;
  double t = nTable.getMaxAge().dbl();
  EXPECT_EQ(t, 0);
}


class NeighborhoodTableTest : public BaseOppTest {
 public:
    NeighborhoodTableTest() {}

    BeaconReceptionInfo*  apply(NeighborhoodTable& table, int nodeId, simtime_t t1, simtime_t t2, inet::Coord c1, inet::Coord c2){
        BeaconReceptionInfo* info = new BeaconReceptionInfo();
        info->setNodeId(nodeId);
        info->initAppData();
        updateInfo(info, t1, t2, c1, c2);

        table.saveInfo(info);
        return info;
    }

    void updateInfo( BeaconReceptionInfo* info, simtime_t t1, simtime_t t2, inet::Coord c1, inet::Coord c2){
        auto data = info->getCurrentDataForUpdate();
        data->setCreationTimeStamp((uint32_t)t1.inUnit(SimTimeUnit::SIMTIME_MS));
        data->setCreationTime(t1);
        data->setReceivedTime(t2);
        data->setPosition(c1);
        data->setEpsilon(c2);

    }


    std::map<int, BeaconReceptionInfo*> infoMap;
};

TEST_F(NeighborhoodTableTest, checkAllTimeToLive) {
  setSimTime(20.0); // set some positive simtime (to keep test values positve)
  simtime_t now = simTime().dbl(); // now == 20.0s
  double maxAge = 3.0;
  NeighborhoodTable nTable;
  nTable.setMaxAge(maxAge);

  //valid, must be present after checkTimeToLive() call
  apply(nTable, 0, now - maxAge + 2, now - maxAge + 2, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0));
  apply(nTable, 1, now - maxAge + 1, now - maxAge + 1, inet::Coord(1.0,0.0), inet::Coord(0.0,0.0));
  apply(nTable, 2, now - maxAge + 0, now - maxAge + 0, inet::Coord(0.0,1.0), inet::Coord(0.0,0.0));

  //invalid, must be removed by checkTimeToLive() call
  apply(nTable, 3, now - maxAge - 1, now - maxAge - 1, inet::Coord(1.0,1.0), inet::Coord(0.0,0.0));
  apply(nTable, 4, now - maxAge - 2, now - maxAge - 2, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0));

  nTable.checkAllTimeToLive();
  EXPECT_TRUE(5 > nTable.getTable().size());

  // check table entries
  EXPECT_EQ(nTable.getTable().count(0), 1);
  EXPECT_EQ(nTable.getTable().count(1), 1);
  EXPECT_EQ(nTable.getTable().count(2), 1);
  EXPECT_EQ(nTable.getTable().count(3), 0); // invalid not found
  EXPECT_EQ(nTable.getTable().count(4), 0); // invlaid not found
}

TEST_F(NeighborhoodTableTest, handleBeacon) {
  NeighborhoodTable nTable;

  // add new entry
  BeaconReceptionInfo* b0 = apply(nTable, 0, 1.0, 2.0, inet::Coord(0.0,0.0), inet::Coord(0.0,0.0));
  EXPECT_EQ(nTable.getTable().size(), 1);
  EXPECT_EQ(nTable.getTable().find(0)->second->getCurrentData()->getCreationTime(), b0->getCurrentData()->getCreationTime());
  EXPECT_EQ(nTable.getTable().find(0)->second->getPrioData(), nullptr);

  // add new entry
  BeaconReceptionInfo* b1  = apply(nTable, 1, 1.0, 3.0, inet::Coord(0.0,1.0), inet::Coord(0.0,0.0));
  EXPECT_EQ(nTable.getTable().size(), 2);
  EXPECT_EQ(nTable.getTable().find(0)->second->getCurrentData()->getCreationTime(), b0->getCurrentData()->getCreationTime());
  EXPECT_EQ(nTable.getTable().find(1)->second->getCurrentData()->getCreationTime(), b1->getCurrentData()->getCreationTime());

  // add entry from existing node id (override old value because new value is younger)
  BeaconReceptionInfo* b2 = nTable.getForUpdate(0);
  updateInfo(b2, 2.0, 4.0, inet::Coord(2.0,0.0), inet::Coord(0.0,0.0));
  EXPECT_EQ(nTable.getTable().size(), 2); // size must not increase
  EXPECT_EQ(nTable.getTable().find(0)->second->getCurrentData()->getCreationTime(), b2->getCurrentData()->getCreationTime());
  EXPECT_EQ(nTable.getTable().find(1)->second->getCurrentData()->getCreationTime(), b1->getCurrentData()->getCreationTime());

}

TEST_F(NeighborhoodTableTest, handleMessage) {
  MockNeighborhoodTable mock;
  cMessage* ttl_msg = new cMessage("TitleMessage");
  cMessage* invalid_msg = new cMessage("InvalidMessage");
  simtime_t time = simTime().dbl();
  double maxAge = 1.0;
  // important to define mock spies before invoking the function to test
  EXPECT_CALL(mock, checkAllTimeToLive())
    .Times(1);
  EXPECT_CALL(mock, scheduleAt(time + maxAge, ttl_msg))
    .Times(1);
  mock.setTitleMessage(ttl_msg);
  mock.setMaxAge(maxAge);
  mock.handleMessage(ttl_msg);
  // test expected exception is thrown
  EXPECT_THROW(mock.handleMessage(invalid_msg), cRuntimeError);
  delete invalid_msg;
}

//TEST_F(NeighborhoodTableTest, initialize) {
//  MockNeighborhoodTable mock;
//  simtime_t time = simTime().dbl();
//  setSimTime(time);
//  double maxAge = 1.0;
//  mock.setMaxAge(maxAge);
//
//  cDoubleParImpl* dPar = new cDoubleParImpl();
//  dPar->setName("maxAge");
//  dPar->setDoubleValue(maxAge);
//  mock.addPar(dPar);
//
//  mock.setMaxAge(maxAge);
//
//  mock.initialize(0); // INITSTAGE_LOCAL
//  mock.initialize(1);
//  mock.initialize(42);
//  EXPECT_CALL(mock, scheduleAt(time + maxAge, testing::_)).Times(1); // triggers on initialize(0)
//
//}
