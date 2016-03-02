/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WEBCONFIRMATION_H_
#define WEBCONFIRMATION_H_ 1

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <string>

#include "core/AbstractWebEngine/TabId.h"

namespace tizen_browser {
namespace basic_webengine {

class WebConfirmation;
typedef std::shared_ptr<WebConfirmation> WebConfirmationPtr;

class WebConfirmation
{
public:
    enum ConfirmationType {
        UserMedia,
        ContentHandler,
        ProtocolHandler,
        Geolocation,
        Notification,
        ScriptAlert,
        ScriptConfirmation,
        ScriptPrompt,
        CertificateConfirmation,
        Authentication
    };

    enum ConfirmationResult {
        None,
        Confirmed,
        Rejected
    };

    WebConfirmation(ConfirmationType type, TabId tabId, const std::string & uri, const std::string & msg);

    virtual ~WebConfirmation();

    ConfirmationType getConfirmationType() const {
        return m_confirmationType;
    }
    TabId getTabId() const {
        return m_tabId;
    }
    std::string getURI() const{
        return m_uri;
    }
    std::string getMessage() const {
        return m_message;
    }
    ConfirmationResult getResult() const {
        return m_result;
    }
    void setResult(ConfirmationResult res) {
        m_result = res;
    }

    virtual bool operator==(const WebConfirmation & n) const {
        return m_confirmationId == n.m_confirmationId;
    }
    virtual bool operator!=(const WebConfirmation & n) const {
        return m_confirmationId != n.m_confirmationId;
    }


private:
    ConfirmationType m_confirmationType;
    boost::uuids::uuid m_confirmationId;
    TabId m_tabId;
    std::string m_uri;
    std::string m_message;
    ConfirmationResult m_result;
};

class AuthenticationConfirmation;
typedef std::shared_ptr<AuthenticationConfirmation> AuthenticationConfirmationPtr;

class AuthenticationConfirmation : public WebConfirmation {
public:
    AuthenticationConfirmation(TabId tabId, const std::string & uri, const std::string & msg);

    void setLogin(const std::string &login) {
    	m_login = login;
    }
    std::string getLogin() const {
    	return m_login;
    }

    void setPassword(const std::string &pass) {
    	m_password = pass;
    }
    std::string getPassword() const {
    	return m_password;
    }

private:
    std::string m_login;
    std::string m_password;
};

class CertificateConfirmation;
typedef std::shared_ptr<CertificateConfirmation> CertificateConfirmationPtr;

class CertificateConfirmation : public WebConfirmation {
public:
	CertificateConfirmation(TabId tabId, const std::string & uri, const std::string & msg);

    void setPem(const std::string &pem) {
    	m_pem = pem;
    }
    std::string getPem() const {
    	return m_pem;
    }

private:
    std::string m_pem;
};

class ScriptPrompt;
typedef std::shared_ptr<ScriptPrompt> ScriptPromptPtr;

class ScriptPrompt : public WebConfirmation {
public:
	ScriptPrompt(TabId tabId, const std::string & uri, const std::string & msg);

    void setUserData(const std::string &userData) {
    	m_userData = userData;
    }
    std::string getUserData() const {
    	return m_userData;
    }

private:
    std::string m_userData;
};


} /* namespace basic_webengine */
} /* namespace tizen_browser */
#endif /* WEBCONFIRMATION_H_ */
