#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
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

#include <set>
#include <vector>

#include "filter/Filter.h"
#include "utils/SortUtils.h"
#include "utils/StdString.h"
#include "utils/XBMCTinyXML.h"

class CDatabase;
class CVariant;

class ISmartPlaylistRule
{
public:
  virtual ~ISmartPlaylistRule() { }

  virtual bool Load(TiXmlElement *element, const CStdString &encoding = "UTF-8") = 0;
  virtual bool Load(const CVariant &obj) = 0;
  virtual bool Save(TiXmlNode *parent) const = 0;
  virtual bool Save(CVariant &obj) const = 0;
};

class CSmartPlaylistRule : public ISmartPlaylistRule
{
public:
  CSmartPlaylistRule();

  enum FIELD_TYPE { TEXT_FIELD = 0,
                    BROWSEABLE_FIELD,
                    NUMERIC_FIELD,
                    DATE_FIELD,
                    PLAYLIST_FIELD,
                    SECONDS_FIELD,
                    BOOLEAN_FIELD,
                    TEXTIN_FIELD
                  };

  virtual bool Load(TiXmlElement *element, const CStdString &encoding = "UTF-8");
  virtual bool Load(const CVariant &obj);
  virtual bool Save(TiXmlNode *parent) const;
  virtual bool Save(CVariant &obj) const;

  CStdString                  GetWhereClause(const CDatabase &db, const CStdString& strType) const;
  static Field                TranslateField(const char *field);
  static CStdString           TranslateField(Field field);
  static SortBy               TranslateOrder(const char *order);
  static CStdString           TranslateOrder(SortBy order);
  static CStdString           GetField(Field field, const CStdString& strType);
  static CStdString           TranslateOperator(CFilterOperator oper);

  static CStdString           GetLocalizedField(Field field);
  static CStdString           GetLocalizedOrder(SortBy order);
  static CStdString           GetLocalizedOperator(CFilterOperator oper);
  static std::vector<Field>   GetFields(const CStdString &type);
  static std::vector<SortBy>  GetOrders(const CStdString &type);
  static FIELD_TYPE           GetFieldType(Field field);

  CStdString                  GetLocalizedRule() const;
  CStdString                  GetParameter() const;
  void                        SetParameter(const CStdString &value);
  void                        SetParameter(const std::vector<CStdString> &values);

  Field                       m_field;
  CFilterOperator             m_operator;
  std::vector<CStdString>     m_parameter;
private:
  static CFilterOperator TranslateOperator(const char *oper);

  CStdString GetVideoResolutionQuery(const CStdString &parameter) const;
};

class CSmartPlaylistRuleCombination;

typedef std::vector<CSmartPlaylistRule> CSmartPlaylistRules;
typedef std::vector<CSmartPlaylistRuleCombination> CSmartPlaylistRuleCombinations;

class CSmartPlaylistRuleCombination : public ISmartPlaylistRule
{
public:
  CSmartPlaylistRuleCombination();

  typedef enum {
    CombinationOr = 0,
    CombinationAnd
  } Combination;

  virtual bool Load(TiXmlElement *element, const CStdString &encoding = "UTF-8") { return false; }
  virtual bool Load(const CVariant &obj);
  virtual bool Save(TiXmlNode *parent) const { return false; }
  virtual bool Save(CVariant &obj) const;

  CStdString GetWhereClause(const CDatabase &db, const CStdString& strType, std::set<CStdString> &referencedPlaylists) const;
  std::string TranslateCombinationType() const;

  Combination GetType() const { return m_type; }
  void SetType(Combination combination) { m_type = combination; }

  void AddRule(const CSmartPlaylistRule &rule);
  void AddCombination(const CSmartPlaylistRuleCombination &rule);

private:
  friend class CSmartPlaylist;
  friend class CGUIDialogSmartPlaylistEditor;
  friend class CGUIDialogMediaFilter;

  Combination m_type;
  CSmartPlaylistRuleCombinations m_combinations;
  CSmartPlaylistRules m_rules;
};

class CSmartPlaylist
{
public:
  CSmartPlaylist();

  bool Load(const CStdString &path);
  bool Load(const CVariant &obj);
  bool LoadFromXml(const CStdString &xml);
  bool LoadFromJson(const CStdString &json);
  bool Save(const CStdString &path) const;
  bool Save(CVariant &obj, bool full = true) const;
  bool SaveAsJson(CStdString &json, bool full = true) const;

  TiXmlElement *OpenAndReadName(const CStdString &path);
  bool LoadFromXML(TiXmlElement *root, const CStdString &encoding = "UTF-8");

  void Reset();

  void SetName(const CStdString &name);
  void SetType(const CStdString &type); // music, video, mixed
  const CStdString& GetName() const { return m_playlistName; };
  const CStdString& GetType() const { return m_playlistType; };

  void SetMatchAllRules(bool matchAll) { m_ruleCombination.SetType(matchAll ? CSmartPlaylistRuleCombination::CombinationAnd : CSmartPlaylistRuleCombination::CombinationOr); }
  bool GetMatchAllRules() const { return m_ruleCombination.GetType() == CSmartPlaylistRuleCombination::CombinationAnd; }

  void SetLimit(unsigned int limit) { m_limit = limit; };
  unsigned int GetLimit() const { return m_limit; };

  void SetOrder(SortBy order) { m_orderField = order; };
  SortBy GetOrder() const { return m_orderField; };

  void SetOrderAscending(bool orderAscending) { m_orderDirection = orderAscending ? SortOrderAscending : SortOrderDescending; };
  bool GetOrderAscending() const { return m_orderDirection != SortOrderDescending; };
  SortOrder GetOrderDirection() const { return m_orderDirection; }

  /*! \brief get the where clause for a playlist
   We handle playlists inside playlists separately in order to ensure we don't introduce infinite loops
   by playlist A including playlist B which also (perhaps via other playlists) then includes playlistA.
   
   \param db the database to use to format up results
   \param referencedPlaylists a set of playlists to know when we reach a cycle
   \param needWhere whether we need to prepend the where clause with "WHERE "
   */
  CStdString GetWhereClause(const CDatabase &db, std::set<CStdString> &referencedPlaylists) const;

  CStdString GetSaveLocation() const;

  static void GetAvailableFields(const std::string &type, std::vector<std::string> &fieldList);

  bool IsEmpty(bool ignoreSortAndLimit = true) const;
private:
  friend class CGUIDialogSmartPlaylistEditor;
  friend class CGUIDialogMediaFilter;

  TiXmlElement* readName();
  TiXmlElement *readNameFromXml(const CStdString &xml);
  bool load(TiXmlElement *root);

  CSmartPlaylistRuleCombination m_ruleCombination;
  CStdString m_playlistName;
  CStdString m_playlistType;

  // order information
  unsigned int m_limit;
  SortBy m_orderField;
  SortOrder m_orderDirection;

  CXBMCTinyXML m_xmlDoc;
};

