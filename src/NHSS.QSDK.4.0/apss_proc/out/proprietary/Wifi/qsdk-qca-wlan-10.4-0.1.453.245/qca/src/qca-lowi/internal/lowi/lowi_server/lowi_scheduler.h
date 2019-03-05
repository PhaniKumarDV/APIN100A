/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

        LOWI Scheduler

GENERAL DESCRIPTION
  This file contains the class definition for the LOWIScheduler
  used to handle periodic scan requests

Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef __LOWI_SCHEDULER_H__
#define __LOWI_SCHEDULER_H__

#include "lowi_scan_measurement.h"
#include "lowi_client.h"
#include "lowi_const.h"
#include "lowi_mac_address.h"
#include "lowi_request.h"
#include "lowi_local_msg.h"

namespace qc_loc_fw
{
// forward declarations
class LOWIController;
class TimerData;
class TimerCallback;

/** defines the states of a wifi node as it's processed by the scheduler */
enum eWiFiNodeState
{
  WIFI_NODE_MEAS_IN_PROGRESS = 0,
  WIFI_NODE_READY_FOR_REQ = 1,
  WIFI_NODE_READY_FOR_RSP = 2,
  WIFI_NODE_WAITING_FOR_TIMER = 3,
  WIFI_NODE_DONE = 4
};

/** strings used for debugging purposes */
static const char* WIFI_NODE_STATE[WIFI_NODE_DONE + 1] =
{
  "WIFI_NODE_MEAS_IN_PROGRESS",
  "WIFI_NODE_READY_FOR_REQ",
  "WIFI_NODE_READY_FOR_RSP",
  "WIFI_NODE_WAITING_FOR_TIMER",
  "WIFI_NODE_DONE"
};

/** context structure for each wifi node being handled by the scheduler */
struct WiFiNodeInfo
{
  //******************************
  // Original request fields
  //******************************
  /** original client request that this wifi node came in */
  LOWIRequest* origReq;
  /** node info from orignal request */
  LOWIPeriodicNodeInfo nodeInfo;

  //******************************
  // Management fields
  //******************************
  /** request id assigned by lowi */
  uint32 schReqId;
  /** RSSI from current try */
  uint16 rssi;
  /** RSSI from last try */
  uint16 lastRssi;
  /** BW from last try */
  eRangingBandwidth lastBw;
  /** periodic counter tracks num measurements left to perform */
  int32 periodicCntr;
  /** retries counter tracks num retries left to perform */
  int32 retryCntr;
  /** state of current wifi node */
  eWiFiNodeState nodeState;
  /** time left before node is placed on a request again.
    * this applies mainly to periodic nodes and gets adjusted
    * when the current timer expires.
    */
  int32 time2ReqMsec;

  //******************************
  // Response fields
  //******************************
  /**
   *  measurement info for this wifinode is stored here when response for
   *  node comes back
   */
  LOWIMeasurementResult* measResult;
  /** measurement results for this particular wifi node */
  LOWIScanMeasurement* meas;

public:
  /**
   * Constructor
   *
   * @param n: periodic node
   * @param req: original request in which node arrived
   */
  WiFiNodeInfo(LOWIPeriodicNodeInfo *n, LOWIRequest *req);
};

/** This struct contains client specific information used as follows:
  * -- to keep track of client requests until they are fully serviced
  * -- to group the wifi nodes by client when these need to be put in a rsp to
  *    the client
  * -- to keep additional measurement result information used in a rsp to the
  *    client
  */
struct LOWIClientInfo
{
  LOWIRequest *clientReq;
  vector<LOWIScanMeasurement *> scanMeasVec;
  LOWIMeasurementResult *result;
  LOWIInternalMessage *iReq;

public:
  /**
   * Constructor
   *
   * @param req: original LOWI request to be stored
   */
  LOWIClientInfo(LOWIRequest *req);

  /**
   * Saves the LOWIInternalMessage in the client list next to the LOWIRequest
   * generated by the scheduler.
   * @param iReq: LOWIInternalMessage
   */
  void saveIReq(LOWIInternalMessage *iReq);
};

/**
 * This class handles requests that have nodes with periodicity or retries.
 * It stores the nodes in a list (aka: data base) and proceeds to schedule the requests
 * into the wifi driver at the appropriate times indicated by a timer.
 * The scheduler handles three kinds of requests:
 * -- periodic ranging scan requests
 * -- ranging scan requests with NAN wifi nodes in them
 * -- cancel requests
 * The scheduler also manages the responses for these requests. Any other type of request/
 * response is not managed by the scheduler.
 * The wifi nodes in the requests may come from multiple clients. The scheduler ensures
 * that the responses are delivered to the appropriate client.
 * The scheduler will keep a list of client requests. Any given client request will be
 * deleted only when all the nodes in the request have been fully serviced.
 */
class LOWIScheduler
{
private:
    static char const *const TAG;
    static char const *const LOWI_SCHED_ORIGINATOR_TAG;

  /** timeout to which the scheduling timer will be set */
    uint32 mCurrTimeoutMsec;

  /**
   * keeps track of whether a timer is currently running and it ensures that
   * timer is created appropriately in multiple places
   */
    bool   mTimerRunning;

  /** list of all wifi nodes that the scheduler is managing */
    List<WiFiNodeInfo*> mNodeDataBase;

  /**
   * List of structures containing client information.
   * The scheduler keeps a list of client requests so that all
   * clients can be fully serviced before the request is
   * removed.
   */
    List<LOWIClientInfo *> mClients;

  /** timer data for the scheduler timer */
    TimerData* mSchedulerTimerData;

  /** Provides access to the lowi-controller functionality */
    LOWIController* mController;

  /**
   * keeps track of how many periodic nodes are in the data base an any
   * given time
   */
    uint32 mNumPeriodicNodesInDB;

  /** time at which timer started */
    uint64 mTimerStarted;

    /**
     * Prints the contents of wifi node database
     */
    void printDataBase();

  /**
   * This function manages the request before sending it to the wifi
   * driver. This applies to periodic and non-periodic ranging requests
   * only. These requests will be scheduled and their periodicity and
   * retries will be managed here. For any other request, this will be a
   * pass through.
   *
   * @param req
   *
   * @return
   * -1: error during request processing
   *  0: request managed
   *  1: request not managed
   */
  int32 manageRequest(LOWIRequest *req);

  /**
   * This function manages a response to a ranging request that is handled by
   * the scheduler and that has a successful status outcome
   *
   * @param meas_result: measurement results to be sent back to the client
   */
  void manageRangRsp(LOWIMeasurementResult *meas_result);

  /**
   * This function manages a response whose status is other that
   * SCAN_STATUS_SUCCESS. It processes the nodes accordingly and
   * sends the response to the client.
   *
   * @param measResult: measurement results
   *  */
  void manageErrRsp(LOWIMeasurementResult *measResult);

  /**
   * This function iterates over the bssid data base and puts together a request
   */
  void setupRequest();

  /**
   * This function iterates over the bssid data base and puts together a
   * response
   */
  void setupResponse();

  /**
   * This function checks if there are periodic wifi nodes in the data base
   *
   * @return uint32: the number of periodic nodes currently in the data base
   */
  uint32 foundPeriodicNodes();

  /**
   *  This function checks if there are NAN wifi nodes in the data base
   *
   * @return uint32: the number of NAN nodes in the data base
   */
  uint32 foundNanNodesInDB();

  /**
   * This function checks the wifi nodes in a RANGING_SCAN and determine if
   * there are NAN nodes
   *
   * @return bool: true if the RANGING_SCAN contains NAN nodes, else false.
   */
  bool foundNanNodesInReq();

  /**
   * Collects all the NAN nodes into a LOWIRequest and sends them to the wifi
   * driver
   */
  void setupNanRequest();

  /**
   * Takes NAN nodes out of a RANGING_SCAN and puts them on the wifi node data
   * base
   */
  void processNanNodes();

  /**
   * This function starts a timer with timeout given by
   * mCurrTimeoutMsec
   *
   * @return int32: 0: timer started successfully, else: failed to
   *         start the timer
   */
  int32 startTimer();

  /**
   * This function computes the new timer period after the timer expires. It
   * iterates over the periodic nodes in the data base recalculating the time
   * left before each node needs to go on a request (time2ReqMsec time). The next
   * timer period is the minimum of these times.
   */
  void computeTimerPeriod();

  /**
   * This function checks to see if the current timer should be stopped when a
   * request arrives that contains periodic nodes while a timer is ongoing. This
   * can be critical when the current timer period is long compared to those in
   * the request. By making this adjustment, the scheduler guarantees that the
   * new arrived nodes will not sit for too long waiting for the current timer
   * to expire. There are two conditions under which the timer will not be
   * adjusted:
   * 1. if the time left on the timer is less than some acceptable wait time (as
   * defined in ACCEPTABLE_WAIT_TIME), the timer will be left to expires
   * 2. if the time left is higher than the ACCEPTABLE_WAIT_TIME, but it less
   * that the new minimum period as found in findNewMinPeriod()
   *
   * @param timeElaped: time elapsed since timer began when new request arrives
   * @param newMinTimerPeriod: new minimum time period
   *
   * @return int32: 0 if ok, else: unable to start a new timer
   */
  int32 adjustTimerPeriod(uint32 timeElaped, uint32 newMinTimerPeriod);

  /**
   * Cycle through the node data base and check the nodes that are ready for
   * request and periodic. Those are the ones that just came in. Find the
   * minimum period among those nodes. This can, potentially, be the new timer
   * period.
   *
   * @param newMinTimerPeriod: variable where the new minimum period will be
   *                         placed
   */
  void findNewMinPeriod(uint32 &newMinTimerPeriod);

  /**
   *  Adjusts the time2ReqMsec for each wifi node in the data base that is
   *  waiting for the timer to expires so that these wifi nodes don't pay a
   *  penalty because the timer is being reset.
   *
   * @param timeMsec: number of msec by which the adjustment will be made
   */
  void adjustTimeLeft(uint32 timeMsec);

/**
   * It changes the status of a wifi node to WIFI_NODE_READY_FOR_REQ. Every wifi node
   * which falls withing the "acceptable" ready time will be set to
   * WIFI_NODE_READY_FOR_REQ. Which nodes will not be known until the minimum time
   * has been calculated.
   */
  void setNodesToReady();

  /**
   * generate a lowi-request id to be used between lowi and the FW. this will
   * enable lowi to send wifi nodes from different clients in a single request
   * to the FW. The algorithm for generating a request id uses the process id in
   * conjunction with an integer which is incremented sequentially.  The process
   * id is placed at the most significant 16 bits of the id and the sequential
   * integer at the least significant 16 bits.
   *
   * @return uint32: request id created.
   */
  uint32 createSchedulerReqId();

  /**
   * prints the request id as a string in a more readable format
   *
   * @param reqId: request id to be printed
   */
  void printSchReqId(uint32 reqId);

  /**
   * This function adds periodic nodes found in a periodic ranging request to
   * the wifi node data base maintained by the scheduler. Once there, the nodes
   * are scheduled on a request according to their period.
   *
   * @param req: original LOWI request containing the nodes to be added to the
   *           database
   */
  void addPeriodicNodesToDB(LOWIRequest *req);

  /**
   * Updates node specific information in the database for the nodes in the
   * reqeust. Specifically, it saves the request id used by the scheduler to
   * handle the request with the FW, and it changes the state of the node to
   * WIFI_NODE_MEAS_IN_PROGRESS
   *
   * @param req: LOWI request containing the wifi nodes to be updated
   */
  void updateNodeInfoInDB(LOWIRequest *req);

  /**
   * Process a wifi node found in the data base using the scan
   * measurement info just received. The processing entails:
   * -- checking it's state and changing it if appropriate
   * -- updating the retry counter if appropriate
   * -- updating the periodic counter if appropriate
   *
   * @param bssid: wifi node mac address
   * @param retry_step: amount by which retry counter will be decremented
   * @param periodic_step: amt by which the periodic cntr will be decremented
   * @param nextState: next wifi node state
   * @param scan_status: status of the response to the ranging request. Used
   *                   to send the appropriate msg to the client and to manage
   *                   the wifi nodes in the data base. For instance, if the
   *                   status shows tha WiFi is off, then retries stop.
   */
  void processNode(LOWIScanMeasurement *node,
                   int32 retry_step,
                   int32 periodic_step,
                   eWiFiNodeState nextState,
                   LOWIResponse::eScanStatus scan_status);

  /**
   * Process a wifi node found in the data base using its bssid. The processing
   * entails:
   * -- checking it's state and changing it if appropriate
   * -- updating the retry counter if appropriate
   * -- updating the periodic counter if appropriate
   *
   * @param bssid: wifi node mac address
   * @param retry_step: amount by which retry counter will be decremented
   * @param periodic_step: amt by which the periodic cntr will be decremented
   * @param nextState: next wifi node state
   * @param scan_status: status of the response to the ranging request. Used
   *                   to send the appropriate msg to the client and to manage
   *                   the wifi nodes in the data base. For instance, if the
   *                   status shows tha WiFi is off, then retries stop.
   *  */
  void processNode(LOWIMacAddress bssid,
                     int32 retry_step,
                     int32 periodic_step,
                   eWiFiNodeState nextState,
                   LOWIResponse::eScanStatus scan_status);

  /**
   * Checks if wifi node is in the data base and retrieve metrics if it is
   *
   * @param bssid: mac address of wifi node
   * @param retries: true if node requires retries, else false
   * @param periodic: true if the node is periodic, else false
   * @param measResult: measurement results to be stored for this wifi node
   *
   * @return bool: true if wifi node is in the data base, else false
   */
  bool isWiFiNodeInDB(LOWIMacAddress bssid,
                      bool &retries,
                      bool &periodic,
                      LOWIMeasurementResult *measResult);

  /**
   * Checks if wifi node is in the data base and retrieve metrics if it is
   *
   * @param bssid: mac address of wifi node
   * @param retries: true if node requires retries, else false
   * @param periodic: true if the node is periodic, else false
   * @param req: request which includes the wifi nodes to be found in the DB
   *
   * @return bool: true if wifi node is in the data base, else false
   */
    bool isWiFiNodeInDB(LOWIMacAddress bssid,
                        bool &retries,
                        bool &periodic,
                        LOWIRequest *req);

  /**
   * This function sends ranging responses to the clients. It configures a
   * response using the nodes for each client that are ready for response and
   * sends the response the client
   */
  void processRangRsp();

  /**
   * Removes from the data base any wifi nodes that have been fully serviced
   */
  void dataBaseCleanup();

  /**
   *  This function is called when a CANCEL_RANGING_SCAN request is received. It
   *  removes any wifi nodes from the data base that are included in the
   *  request.
   *
   * @param req: LOWI request containing the wifi nodes to be cancelled
   */
  void processCancelReq(LOWIRequest *req);

  /**
   * Removes client information that is no longer useful or needed.
   * -- removes the ranging requests that have already been fully serviced
   * -- removes all response data that was already sent to clients
   */
  void deleteClientInfo();

  /**
   * General check to ensure that the ranging request is valid
   *
   * @param req: LOWIrequest to be checked
   * @return bool: true if request is valid, else false
   */
  bool isReqOk(LOWIRequest* req);

  /**
   * This function does general clean up:
   * -- it calls a specific function "dataBaseCleanup" to remove serviced nodes
   *    from the data base
   * -- it removes client requests that have been fully serviced
   * -- it removes the timer if no more periodic nodes exit
   */
  void cleanUp();

  /**
   * determines if there are wifi nodes that need to be returned to clients in a
   * response
   *
   * @return bool: true if there are wifi nodes to be returned to client in a
   *         rsp, else false.
   */
  bool nodesForRsp();

  /**
   * stores specific node information into the list mClients that will be used
   * in the response to the client
   *
   * @param pNode: measurement specific data from this node that will be stored
   *               by this function
   * @param scan_status: status of the response to the ranging request. Used
   *                     to send the appropriate msg to the client and to
   *                     manage the wifi nodes in the data base. For instance,
   *                     if the status shows tha WiFi is off, then retries
   *                     stop.
   */
  void addToClientList(WiFiNodeInfo *pNode, LOWIResponse::eScanStatus scan_status);

  /**
   * Searches the list of active client requests and saves the internal message
   * iReq. Used to keep track of which client requests were generated from
   * internal messages.
   *
   * @param req: LOWIRequest generated by the scheduler
   * @param iReq: original internal message
   */
  void saveIReq(LOWIRequest *req, LOWIInternalMessage *iReq);

  /**
   * Calculates the range between STA and AP
   *
   * @param measInfo: rtt measurements
   * @return uint32: range in 1/64 m units
   */
  uint32 calculateRangeForFTMRR(vector <LOWIMeasurementInfo *> &measInfo);

  /**
   * Process an LOWIInternalMessage
   * @param client: client class containing message
   */
  void processInternalMsg(LOWIClientInfo *client);

public:
  /**
   * Constructor takes a lowi-controller object so that the scheduler can call
   * lowi-controller functions
   *
   * @param pController : lowi controller object
   */
  LOWIScheduler(LOWIController* pController);
  ~LOWIScheduler();

  /**
   * This function is called by the lowi controller. It's the entry point into
   * the scheduler. It takes the input message into the lowi controller and
   * passes it to the lowi scheduler to process the ranging requests and
   * responses
   *
   * @param msg : message to be handled
   *
   *@return bool : true: scheduler managed the msg, false: scheduler did not
   *         managed the msg
   */
  bool manageMsg(LOWILocalMsg *msg);

  /**
   * Called when the timer expires. If there are more periodic
   * nodes in the data base, this function will compute a new timer period, set
   * the appropriate nodes to ready and set up a new ranging scan request with
   * those nodes that are ready for request.
   */
  void timerCallback();

  /**
   * checks to see if the request was originated by the scheduler
   *
   * @param req: LOWI request to check
   *
   * @return bool: true if request was originated by the scheduler, else false
   */
  bool isSchedulerRequest(const LOWIRequest *req);

  /**
   * Handles a request that was not successfully sent to the wifi driver. This
   * could happen if the wifi gets turned off, for instance, while the scheduler
   * is trying to send a request.
   *
   * @param req: LOWI request that could not be sent
   * @param scan_status: scan status used to manage the rsp to the client and
   *                   the nodes in the wifi database
   */
  void manageErrRsp(LOWIRequest *req, LOWIResponse::eScanStatus scan_status);
};
} // namespace qc_loc_fw

#endif // __LOWI_SCHEDULER_H__
