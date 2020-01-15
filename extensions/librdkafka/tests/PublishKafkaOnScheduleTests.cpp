/**
 * @file GenerateFlowFile.h
 * GenerateFlowFile class declaration
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#undef NDEBUG
#include <cassert>
#include "../../../libminifi/test/integration/IntegrationBase.h"
#include "core/logging/Logger.h"
#include "../../../libminifi/test/TestBase.h"
#include "../PublishKafka.h"

class PublishKafkaOnScheduleTests : public IntegrationBase {
public:
    virtual void runAssertions() {
      std::string logs = LogTestController::getInstance().log_output.str();
      size_t pos = 0;
      size_t last_pos = 0;
      unsigned int occurances = 0;
      do {
        pos = logs.find(" value 1 is outside allowed range 1000..1000000000", pos);
        if (pos != std::string::npos) {
          last_pos = pos;
          pos = logs.find("notifyStop called", pos);
          if (pos != std::string::npos) {
            last_pos = pos;
            occurances++;
          }
        }
      } while (pos != std::string::npos);

      assert(occurances > 1);  // Verify retry of onSchedule and onUnSchedule calls

      // Make sure onSchedule succeeded after property was set
      assert(logs.find("Successfully configured PublishKafka", last_pos) != std::string::npos);

      // Make sure onTrigger was called after onshedule succeeded
      pos = logs.find("PublishKafka onTrigger");
      assert(pos != std::string::npos && pos > last_pos);
    }

    virtual void testSetup() {
      LogTestController::getInstance().setDebug<core::ProcessGroup>();
      LogTestController::getInstance().setDebug<core::Processor>();
      LogTestController::getInstance().setDebug<core::ProcessSession>();
      LogTestController::getInstance().setDebug<minifi::processors::PublishKafka>();
    }

    virtual void waitToVerifyProcessor() {
      std::this_thread::sleep_for(std::chrono::seconds(3));
      flowController_->updatePropertyValue("kafka", minifi::processors::PublishKafka::MaxMessageSize.getName(), "1999");
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    virtual void cleanup() {}
};

int main(int argc, char **argv) {
  std::string test_file_location, url;
  if (argc > 1) {
    test_file_location = argv[1];
  }

  PublishKafkaOnScheduleTests harness;

  harness.run(test_file_location);

  return 0;
}
