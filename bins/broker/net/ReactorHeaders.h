//
// Created by Alexander Bychuk on 26.01.2021.
//

#ifndef UPMQ_BINS_BROKER_NET_REACTORHEADERS_H_
#define UPMQ_BINS_BROKER_NET_REACTORHEADERS_H_

#ifdef UPMQ_REACTOR
#include "SocketNotifier.h"
#include "SocketReactor.h"
#include "SocketNotification.h"
#include "ParallelSocketReactor.h"
#include "ParallelSocketAcceptor.h"

namespace PNet = upmq::Net;

#else
#include <Poco/Net/SocketNotifier.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Net/ParallelSocketReactor.h>
#include <Poco/Net/ParallelSocketAcceptor.h>

namespace PNet = Poco::Net;

#endif

#endif  // UPMQ_BINS_BROKER_NET_REACTORHEADERS_H_
