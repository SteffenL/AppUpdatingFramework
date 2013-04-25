#include "Update.h"
#include <json/json.h>
#include <algorithm>

namespace aufw { namespace web_api { namespace v2 { namespace update {

void CheckProductResult::FromJson(const Json::Value& jsonValue) {
    UpdateDetails.UniqueKey = jsonValue["ukey"].asString();
    UpdateDetails.Version = jsonValue["version"].asString();
    UpdateDetails.DisplayName = jsonValue["display_name"].asString();
    UpdateDetails.Description = jsonValue["description"].asString();
    UpdateDetails.UpdateDownloadUrl = jsonValue["download_url"].asString();
    UpdateDetails.ManualDownloadUrl = jsonValue["manual_download_url"].asString();
    UpdateDetails.InfoUrl = jsonValue["info_url"].asString();
    UpdateDetails.ReleaseNotesUrl = jsonValue["release_notes_url"].asString();
    UpdateDetails.UpdateFileHash = jsonValue["update_file_hash"].asString();
    UpdateDetails.UpdateFileSize = jsonValue["update_file_size"].asUInt();
}

CheckResult::CheckResult() : HasApplicationUpdate(false), HaveComponentUpdates(false) {}

void CheckResult::FromJson(const Json::Value& jsonValue) {
    HasApplicationUpdate = false;
    HaveComponentUpdates = false;

    if (!jsonValue.isMember("updates")) {
        return;
    }

    const Json::Value& jsonUpdates = jsonValue["updates"];
    // No updates?
    if (jsonUpdates.size() == 0) {
        return;
    }

    if (jsonUpdates.isMember("application")) {
        Application.FromJson(jsonUpdates["application"]);
        HasApplicationUpdate = true;
    }

    if (jsonUpdates.isMember("components")) {
        const Json::Value& components = jsonUpdates["components"];
        for (Json::ValueConstIterator it = components.begin(), end(components.end()); it != end; ++it) {
            CheckProductResult r;
            r.FromJson(*it);
            Components.push_back(r);
        }

        HaveComponentUpdates = (Components.size() > 0);
    }
}


void CheckArg::ToJson(const ProductInstallationInfo& productInfo, Json::Value& jsonValue) {
    jsonValue["ukey"] = productInfo.UniqueKey;
    jsonValue["version"] = productInfo.Version.ToString();
}

void CheckArg::ToJson(const std::list<ProductInstallationInfo>& productInfoList, Json::Value& jsonValue) {
    for (std::list<ProductInstallationInfo>::const_iterator it = productInfoList.begin(), end(productInfoList.end()); it != end; ++it) {
        Json::Value newJsonValue;
        ToJson(*it, newJsonValue);
        jsonValue.append(newJsonValue);
    }
}

Update::Update() {}

void Update::Check(const CheckArg& arg, CheckResult& result, const std::string& channel, SearchMethod::type searchMethod) {
    Json::Value jsonData;
    CheckArg::ToJson(arg.Application, jsonData["application"]);
    CheckArg::ToJson(arg.Components, jsonData["components"]);
    jsonData["channel"] = channel;
    jsonData["searchMethod"] = searchMethod;

    Json::Value jsonResponse;
    sendRequest("/api/v2/update/check", jsonData, jsonResponse, "POST");
    //receiveResponse(jsonResponse);
    result.FromJson(jsonResponse);
}


void CheckProductResultList::GetUpdateDetails(std::list<ProductUpdateDetails>& detailsList) {
    std::for_each(begin(), end(), [&](const CheckProductResult& result) {
        detailsList.push_back(result.UpdateDetails);
    });
}

} } } } // namespace
