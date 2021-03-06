//
// Copyright (C) 2019 Linkworld Open Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https: //www.gnu.org/licenses/>.
#ifndef __MOSS_TEST
#define __MOSS_TEST
#endif
#include "./connection.h"
#include "gtest/gtest.h"

TEST(Connection, NewID) {
  std::shared_ptr<moss::Connection> conn(
      new moss::Connection(moss::ConnectionType::kClient));
  EXPECT_EQ(__Test_NewID(conn, moss::Initializer::kClient,
                         moss::Directional::kBidirectional),
            0);
  EXPECT_EQ(__Test_NewID(conn, moss::Initializer::kServer,
                         moss::Directional::kBidirectional),
            5);
  EXPECT_EQ(__Test_NewID(conn, moss::Initializer::kClient,
                         moss::Directional::kUnidirectional),
            10);
  EXPECT_EQ(__Test_NewID(conn, moss::Initializer::kServer,
                         moss::Directional::kUnidirectional),
            15);
}

void TestCreateStream(std::shared_ptr<moss::Connection> conn,
                      const moss::ConnectionType& conn_type,
                      const moss::Directional& direct,
                      moss::streamID_t expectID) {
  auto stream = conn->CreateStream(direct);
  EXPECT_EQ(stream->id_, expectID);
  EXPECT_EQ(stream->initer_, conn_type);
  EXPECT_EQ(stream->direct_, direct);
}

TEST(Connection, CreateStream) {
  std::shared_ptr<moss::Connection> cliConn(
      new moss::Connection(moss::ConnectionType::kClient));
  std::shared_ptr<moss::Connection> svrConn(
      new moss::Connection(moss::ConnectionType::kServer));
  TestCreateStream(cliConn, moss::ConnectionType::kClient,
                   moss::Directional::kBidirectional, 0);
  TestCreateStream(cliConn, moss::ConnectionType::kClient,
                   moss::Directional::kUnidirectional, 6);
  TestCreateStream(svrConn, moss::ConnectionType::kServer,
                   moss::Directional::kUnidirectional, 3);
  TestCreateStream(svrConn, moss::ConnectionType::kServer,
                   moss::Directional::kBidirectional, 5);
}
