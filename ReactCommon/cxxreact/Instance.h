// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <memory>

#include <cxxreact/ModuleRegistry.h>
#include <cxxreact/NativeModule.h>
#include <cxxreact/NativeToJsBridge.h>
#include <folly/dynamic.h>

namespace facebook {
namespace react {

class JSExecutorFactory;

struct InstanceCallback {
  virtual ~InstanceCallback() {}
  virtual void onBatchComplete() = 0;
  virtual void incrementPendingJSCalls() = 0;
  virtual void decrementPendingJSCalls() = 0;
};

class Instance {
 public:
  ~Instance();
  void initializeBridge(
    std::unique_ptr<InstanceCallback> callback,
    std::shared_ptr<JSExecutorFactory> jsef,
    std::shared_ptr<MessageQueueThread> jsQueue,
    std::shared_ptr<ModuleRegistry> moduleRegistry);

  void setSourceURL(std::string sourceURL);

  void loadScriptFromString(
    std::unique_ptr<const JSBigString> string,
    std::string sourceURL,
    bool loadSynchronously);
  void loadUnbundle(
    std::unique_ptr<JSModulesUnbundle> unbundle,
    std::unique_ptr<const JSBigString> startupScript,
    std::string startupScriptSourceURL,
    bool loadSynchronously);
  bool supportsProfiling();
  void startProfiler(const std::string& title);
  void stopProfiler(const std::string& title, const std::string& filename);
  void setGlobalVariable(std::string propName, std::unique_ptr<const JSBigString> jsonValue);
  void *getJavaScriptContext();
  void callJSFunction(std::string&& module, std::string&& method, folly::dynamic&& params);
  void callJSCallback(uint64_t callbackId, folly::dynamic&& params);
  MethodCallResult callSerializableNativeHook(unsigned int moduleId, unsigned int methodId, folly::dynamic&& args);
  // This method is experimental, and may be modified or removed.
  template <typename T>
  Value callFunctionSync(const std::string& module, const std::string& method, T&& args) {
    CHECK(nativeToJsBridge_);
    return nativeToJsBridge_->callFunctionSync(module, method, std::forward<T>(args));
  }

  void handleMemoryPressureUiHidden();
  void handleMemoryPressureModerate();
  void handleMemoryPressureCritical();

 private:
  void callNativeModules(folly::dynamic&& calls, bool isEndOfBatch);
  void loadApplication(
    std::unique_ptr<JSModulesUnbundle> unbundle,
    std::unique_ptr<const JSBigString> startupScript,
    std::string startupScriptSourceURL);
  void loadApplicationSync(
    std::unique_ptr<JSModulesUnbundle> unbundle,
    std::unique_ptr<const JSBigString> startupScript,
    std::string startupScriptSourceURL);

  std::shared_ptr<InstanceCallback> callback_;
  std::unique_ptr<NativeToJsBridge> nativeToJsBridge_;
  std::shared_ptr<ModuleRegistry> moduleRegistry_;

  std::mutex m_syncMutex;
  std::condition_variable m_syncCV;
  bool m_syncReady = false;
};

}
}
