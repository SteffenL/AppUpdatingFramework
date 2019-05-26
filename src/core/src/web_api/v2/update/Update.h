#include "../../ApiController.h"
#include <aufw/core/ProductInstallationInfo.h>
#include <aufw/core/ProductUpdateDetails.h>
#include <aufw/core/SearchMethod.h>
#include <string>
#include <list>

namespace Json { class Value; }

namespace aufw { namespace web_api { namespace v2 { namespace update {

class CheckArg {
public:
    static void ToJson(const ProductInstallationInfo& productInfo, Json::Value& jsonValue);
    static void ToJson(const std::list<ProductInstallationInfo>& productInfoList, Json::Value& jsonValue);

public:
    ProductInstallationInfo Application;
    std::list<ProductInstallationInfo> Components;
};

class CheckProductResult {
public:
    void FromJson(const Json::Value& jsonValue);

public:
    ProductUpdateDetails UpdateDetails;
};

class CheckProductResultList : public std::list<CheckProductResult> {
public:
    void GetUpdateDetails(std::list<ProductUpdateDetails>& detailsList);
};

class CheckResult {
public:
    CheckResult();
    void FromJson(const Json::Value& jsonValue);

public:
    CheckProductResult Application;
    CheckProductResultList Components;
    bool HasApplicationUpdate;
    bool HaveComponentUpdates;
};

class Update : public ApiController {
public:
    Update();
    void Check(const CheckArg& arg, CheckResult& result, const std::string& channel, SearchMethod::type searchMethod);

private:

};

} } } } // namespace
