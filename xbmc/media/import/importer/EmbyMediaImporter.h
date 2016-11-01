#pragma once
/*
 *      Copyright (C) 2016 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <string>

#include "FileItem.h"
#include "media/import/IMediaImporter.h"
#include "threads/Thread.h"

class CVariant;
class CVideoInfoTag;

class CEmbyMediaImporter : public IMediaImporter
{
public:
  CEmbyMediaImporter();
  virtual ~CEmbyMediaImporter();

  // implementations of IMediaImporter
  virtual const char* GetIdentification() const override { return "EmbyMediaImporter"; }

  virtual bool CanImport(const std::string &path) const override;
  virtual bool LoadSourceSettings(CMediaImportSource& source) const override;
  virtual bool UnloadSourceSettings(CMediaImportSource& source) const override;
  virtual bool CanUpdateMetadataOnSource(const std::string &path) const override { return true; }
  virtual bool CanUpdatePlaycountOnSource(const std::string &path) const override { return true; }
  virtual bool CanUpdateLastPlayedOnSource(const std::string &path) const override { return true; }
  virtual bool CanUpdateResumePositionOnSource(const std::string &path) const override { return false; }

  virtual IMediaImporter* Create(const CMediaImport &import) const override;
  virtual void Start() override;
  virtual bool Import(CMediaImportRetrievalTask *task) const override;
  virtual bool UpdateOnSource(CMediaImportUpdateTask* task) const override;

protected:
  CEmbyMediaImporter(const CMediaImport &import);

  CFileItemPtr ToFileItem(const CVariant& item, const MediaType& mediaType) const;
  void FillVideoInfoTag(const CVariant& item, CFileItemPtr fileItem, const MediaType& mediaType, CVideoInfoTag* videoInfo) const;

  bool MarkAsWatched(const std::string& itemId, CDateTime lastPlayed) const;
  bool MarkAsUnwatched(const std::string& itemId) const;
  bool UpdateResumePoint(const std::string& itemId, uint64_t resumePointInTicks) const;

  std::string BuildUrl(const std::string& endpoint) const;
  std::string BuildUserUrl(const std::string& endpoint) const;
  std::string BuildItemUrl(const std::string& itemId) const;
  std::string BuildUserItemUrl(const std::string& itemId) const;
  std::string BuildUserPlayedItemUrl(const std::string& itemId) const;

  std::string BuildPlayableItemPath(const std::string& mediaType, const std::string& itemId, const std::string& container) const;
  std::string BuildFolderItemPath(const std::string& itemId) const;
  std::string BuildImagePath(const std::string& itemId, const std::string& imageType, const std::string& imageTag = "") const;

  bool ApiGet(const std::string& url, std::string& response) const;
  bool ApiPost(const std::string& url, const std::string& data, std::string& response) const;
  bool ApiDelete(const std::string& url, std::string& response) const;

  static bool GetServerId(const std::string& path, std::string& id);
  static bool GetItemId(const std::string& path, std::string& id);

  static void SettingOptionsUsersFiller(const CSetting* setting, std::vector<std::pair<std::string, std::string>>& list, std::string& current, void* data);

  std::string m_serverId;
  std::string m_deviceId;
  std::string m_url;

  class CEmbyAuthenticator
  {
  public:
    CEmbyAuthenticator();
    ~CEmbyAuthenticator() = default;

    static CEmbyAuthenticator WithApiKey(const std::string& serviceUrl, const std::string& deviceId, const std::string& apiKey);
    static CEmbyAuthenticator WithUserId(const std::string& serviceUrl, const std::string& deviceId, const std::string& userId, const std::string& password = "");
    static CEmbyAuthenticator WithUsername(const std::string& serviceUrl, const std::string& deviceId, const std::string& username, const std::string& password = "");

    bool Authenticate() const;
    bool IsAuthenticated() const { return !m_accessToken.empty(); }

    const std::string& GetAccessToken() const { return m_accessToken; }
    const std::string& GetUserId() const { return m_userId; }

  private:
    explicit CEmbyAuthenticator(const std::string& serviceUrl, const std::string& deviceId,
      const std::string& apiKey, const std::string& userId, const std::string& username, const std::string& password);

    enum class AuthenticationMethod
    {
      None = 0,
      ApiKey,
      UserId,
      Username
    };
    AuthenticationMethod m_authMethod;

    std::string m_url;
    std::string m_deviceId;

    std::string m_apiKey;
    std::string m_username;
    std::string m_password;

    mutable std::string m_userId;
    mutable std::string m_accessToken;
  };

  CEmbyAuthenticator m_authenticator;

  class CEmbyServerDiscovery : protected CThread
  {
  public:
    CEmbyServerDiscovery();
    virtual ~CEmbyServerDiscovery() = default;

    void Start();

  protected:
    virtual void Process() override;

  private:
    struct EmbyServer
    {
      std::string id;
      std::string name;
      std::string address;
      bool registered;
      CDateTime lastSeen;
    };

    void AddEmbyServer(const EmbyServer& embyServer);
    void ExpireEmbyServers();

    static bool ToEmbyServer(const std::string& result, EmbyServer& embyServer);

    std::map<std::string, EmbyServer> m_servers;
  };

  CEmbyServerDiscovery m_discovery;
};
