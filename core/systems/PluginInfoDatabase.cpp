#include <stdarg.h>
#include <string.h>
#include "PluginInfoDatabase.h"
#include "PluginSys.h"

void PluginSettings::Init()
{
	name = -1;
	pause_val = false;
	type_val = PluginType_MapUpdated;
	optarray = -1;
	opts_num = 0;
	opts_size = 0;
}

/**
 * :TODO: write the logger, make these errors log instead of being static
 * NOTE: once we do that, we will have to change some code to ignore sections
 */

CPluginInfoDatabase::CPluginInfoDatabase()
{
	m_strtab = NULL;
	m_infodb_count = 0;
	m_infodb_size = 0;
	m_infodb = -1;
}

CPluginInfoDatabase::~CPluginInfoDatabase()
{
	delete m_strtab;
}

void CPluginInfoDatabase::ReadSMC_ParseStart()
{
	/* Create or reset our string table */
	if (m_strtab)
	{
		m_strtab->Reset();
	} else {
		m_strtab = new BaseStringTable(1024);
	}

	/* Set our internal states to the beginning */
	in_plugins = false;
	cur_plugin = -1;
	in_options = false;
	m_infodb_size = 0;
	m_infodb_count = 0;
	m_infodb = -1;
}

SMCParseResult CPluginInfoDatabase::MakeError(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	m_errmsg = m_strtab->AddString(buffer);

	return SMCParse_HaltFail;
}

unsigned int CPluginInfoDatabase::GetSettingsNum()
{
	return m_infodb_count;
}

PluginSettings *CPluginInfoDatabase::GetSettingsIfMatch(unsigned int index, const char *filename)
{
	BaseMemTable *memtab = m_strtab->GetMemTable();
	int *table = (int *)memtab->GetAddress(m_infodb);

	if (!table || index >= m_infodb_count)
	{
		return NULL;
	}

	PluginSettings *plugin = (PluginSettings *)memtab->GetAddress(table[index]);

	const char *name = m_strtab->GetString(plugin->name);

	if (!name)
	{
		return NULL;
	}

	if (!g_PluginSys.TestAliasMatch(name, filename))
	{
		return NULL;
	}

	return plugin;
}

void CPluginInfoDatabase::GetOptionsForPlugin(PluginSettings *settings, unsigned int opt_num, const char **key, const char **val)
{
	PluginOpts *table = (PluginOpts *)m_strtab->GetMemTable()->GetAddress(settings->optarray);
	if (!table)
	{
		*key = NULL;
		*val = NULL;
		return;
	}

	if (opt_num >= settings->opts_num)
	{
		*key = NULL;
		*val = NULL;
		return;
	}

	*key = m_strtab->GetString(table[opt_num].key);
	*val = m_strtab->GetString(table[opt_num].val);
}

SMCParseResult CPluginInfoDatabase::ReadSMC_KeyValue(const char *key, 
													 const char *value, 
													 bool key_quotes, 
													 bool value_quotes)
{
	if (cur_plugin != -1)
	{
		PluginSettings *plugin = (PluginSettings *)m_strtab->GetMemTable()->GetAddress(cur_plugin);
		if (!in_options)
		{
			if (strcmp(key, "pause") == 0)
			{
				if (strcasecmp(value, "yes") == 0)
				{
					plugin->pause_val = true;
				} else {
					plugin->pause_val = false;
				}
			} else if (strcmp(key, "lifetime") == 0) {
				if (strcasecmp(value, "private") == 0)
				{
					plugin->type_val = PluginType_Private;
				} else if (strcasecmp(value, "mapsync") == 0) {
					plugin->type_val = PluginType_MapUpdated;
				} else if (strcasecmp(value, "maponly") == 0) {
					plugin->type_val = PluginType_MapOnly;
				} else if (strcasecmp(value, "global") == 0) {
					plugin->type_val = PluginType_Global;
				} else {
					return MakeError("Unknown value for key \"lifetime\": \"%s\"", value);
				}
			} else {
				return MakeError("Unknown property key: \"%s\"", key);
			}
		} else {
			/* Cache every option, valid or not */
			int keyidx = m_strtab->AddString(key);
			int validx = m_strtab->AddString(value);
			PluginOpts *table;
			BaseMemTable *memtab = m_strtab->GetMemTable();
			if (plugin->opts_num + 1 > plugin->opts_size)
			{
				unsigned int oldsize = plugin->opts_size;
				if (oldsize == 0)
				{
					//right now we don't have many
					plugin->opts_size = 2;
				} else {
					plugin->opts_size *= 2;
				}
				int newidx = memtab->CreateMem(plugin->opts_size * sizeof(PluginOpts), (void **)&table);
				/* in case it resized */
				plugin = (PluginSettings *)memtab->GetAddress(cur_plugin);
				if (plugin->optarray != -1)
				{
					void *oldtable = memtab->GetAddress(plugin->optarray);
					memcpy(table, oldtable, oldsize * sizeof(PluginOpts));
				}
				plugin->optarray = newidx;
			} else {
				table = (PluginOpts *)memtab->GetAddress(plugin->optarray);
			}
			PluginOpts *opt = &table[plugin->opts_num++];
			opt->key = keyidx;
			opt->val = validx;
		}
	} else if (in_plugins) {
		return MakeError("Unknown property key: \"%s\"", key);
	} else {
		/* Ignore anything we don't know about! */
	}

	return SMCParse_Continue;
}

SMCParseResult CPluginInfoDatabase::ReadSMC_LeavingSection()
{
	if (in_plugins)
	{
		if (cur_plugin != -1)
		{
			if (in_options)
			{
				in_options = false;
			} else {
				/* If the plugin is ending, add it to the table */
				BaseMemTable *memtab = m_strtab->GetMemTable();
				int *table;
				if (m_infodb_count + 1 > m_infodb_size)
				{
					unsigned int oldsize = m_infodb_size;
					if (!m_infodb_size)
					{
						m_infodb_size = 8;
					} else {
						m_infodb_size *= 2;
					}
					int newidx = memtab->CreateMem(m_infodb_size, (void **)&table);
					if (m_infodb != -1)
					{
						void *oldtable = (int *)memtab->GetAddress(m_infodb);
						memcpy(table, oldtable, oldsize * sizeof(int));
					}
					m_infodb = newidx;
				} else {
					table = (int *)memtab->GetAddress(m_infodb);
				}
				/* Assign to table and scrap the current plugin */
				table[m_infodb_count++] = cur_plugin;
				cur_plugin = -1;
			}
		} else {
			in_plugins = false;
		}
	}

	return SMCParse_Continue;
}

SMCParseResult CPluginInfoDatabase::ReadSMC_NewSection(const char *name, bool opt_quotes)
{
	if (!in_plugins)
	{
		/* If we're not in the main Plugins section, and we don't get it for the name, error out */
		if (strcmp(name, "Plugins") != 0)
		{
			return MakeError("Unknown root section: \"%s\"", name);
		} else {
			/* Otherwise set our states */
			in_plugins = true;
			cur_plugin = -1;
			in_options = false;
		}
	} else {
		if (cur_plugin == -1)
		{
			/* If we get a plugin node and we don't have a current plugin, create a new one */
			PluginSettings *plugin;
			cur_plugin = m_strtab->GetMemTable()->CreateMem(sizeof(PluginSettings), (void **)&plugin);
			plugin->Init();
			plugin->name = m_strtab->AddString(name);
			in_options = false;
		} else {
			if (!in_options && strcmp(name, "Options") == 0)
			{
				in_options = true;
			} else {
				return MakeError("Unknown plugin sub-section: \"%s\"", name);
			}
		}
	}

	return SMCParse_Continue;
}
