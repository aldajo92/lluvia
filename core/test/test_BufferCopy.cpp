/**
 * \file test_BufferCopy.cpp
 * \brief test buffer copy
 * \copyright 2017, Juan David Adarve. See AUTHORS for more details
 * \license Apache 2.0, see LICENSE for more details
 */

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include <iostream>
#include "lluvia/core.h"

using memflags = vk::MemoryPropertyFlagBits;


TEST_CASE("HostToDeviceToHost", "BufferCopyTest") {

    constexpr const auto length = 128u;
    constexpr const auto bufferSize = length*sizeof(int);

    auto session = ll::Session::create();
    const auto hostMemFlags   = memflags::eHostVisible | memflags::eHostCoherent;;
    const auto deviceMemFlags = memflags::eDeviceLocal;
        
    auto hostMemory = session->createMemory(hostMemFlags, bufferSize, false);
    REQUIRE(hostMemory != nullptr);

    auto hostBuffer = hostMemory->createBuffer(bufferSize);
    REQUIRE(hostBuffer != nullptr);

    auto deviceMemory = session->createMemory(deviceMemFlags, bufferSize, false);
    REQUIRE(deviceMemory != nullptr);

    auto deviceBuffer = deviceMemory->createBuffer(bufferSize);
    REQUIRE(deviceBuffer != nullptr);

    // fill the host buffer with some values
    auto hostPtr = static_cast<int*>(hostMemory->mapBuffer(*hostBuffer));
    REQUIRE(hostPtr != nullptr);
    for (auto i = 0u; i < length; ++i) {
        hostPtr[i] = i;
    }
    hostMemory->unmapBuffer(*hostBuffer);

    // issue the copy command
    session->copyBuffer(*hostBuffer, *deviceBuffer);


    // create a second host memory to copy the deviceBuffer into and check
    // if the content is equal to hostBuffer
    auto secMemory = session->createMemory(hostMemFlags, bufferSize, false);
    REQUIRE(secMemory != nullptr);

    auto secBuffer = secMemory->createBuffer(bufferSize);
    REQUIRE(secBuffer != nullptr);

    session->copyBuffer(*deviceBuffer, *secBuffer);


    // compare host and secondary values. If they are equal, then it
    // means that the memory content of deviceBuffer is also equal.
    hostPtr = static_cast<int*>(hostMemory->mapBuffer(*hostBuffer));
    auto secPtr = static_cast<int*>(secMemory->mapBuffer(*secBuffer));

    REQUIRE(hostPtr != nullptr);
    REQUIRE(secPtr != nullptr);

    auto areEqual = true;
    for (auto i = 0u; i < length; ++i) {

        const auto& hostValue = hostPtr[i];
        const auto& secValue = secPtr[i];

        if (hostValue != secValue) {
            std::cout << "values not equal at: " << i << ": " << hostPtr[i] << " != " << secPtr[i] << std::endl;
            areEqual = false;
            break;    
        }
    }

    REQUIRE(areEqual == true);

    hostMemory->unmapBuffer(*hostBuffer);
    secMemory->unmapBuffer(*secBuffer);
}

