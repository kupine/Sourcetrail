#include "component/controller/SearchController.h"

#include "component/view/SearchView.h"
#include "data/access/StorageAccess.h"
#include "utility/tracing.h"

SearchController::SearchController(StorageAccess* storageAccess)
	: m_storageAccess(storageAccess)
{
}

SearchController::~SearchController()
{
}

void SearchController::handleMessage(MessageActivateTokens* message)
{
	if (!message->isFromSearch)
	{
		if (!message->keepContent() && message->tokenIds.size())
		{
			getView()->setMatches(m_storageAccess->getSearchMatchesForTokenIds(message->tokenIds));
		}
		else if ((message->isReplayed() || message->unknownNames.size()) && !message->tokenIds.size())
		{
			std::vector<SearchMatch> matches;

			for (const std::string& name : message->unknownNames)
			{
				matches.push_back(SearchMatch(name));
			}

			if (!matches.size())
			{
				matches.push_back(SearchMatch("<invalid>"));
			}

			getView()->setMatches(matches);
		}
	}
}

void SearchController::handleMessage(MessageFind* message)
{
	if (message->findFulltext)
	{
		getView()->findFulltext();
	}
	else
	{
		getView()->setFocus();
	}
}

void SearchController::handleMessage(MessageSearch* message)
{
	const std::vector<SearchMatch>& matches = message->getMatches();

	for (const SearchMatch& match : matches)
	{
		if (match.searchType == SearchMatch::SEARCH_COMMAND)
		{
			SearchMatch m = SearchMatch::createCommand(SearchMatch::getCommandType(match.getFullName()));
			getView()->setMatches(std::vector<SearchMatch>(1, m));
			return;
		}
	}
}

void SearchController::handleMessage(MessageSearchAutocomplete* message)
{
	TRACE("search autocomplete");

	LOG_INFO("autocomplete string: \"" + message->query + "\"");
	getView()->setAutocompletionList(m_storageAccess->getAutocompletionMatches(message->query));
}

void SearchController::handleMessage(MessageSearchFullText* message)
{
	LOG_INFO("fulltext string: \"" + message->searchTerm + "\"");
	std::string prefix(message->caseSensitive ? 2 : 1, SearchMatch::FULLTEXT_SEARCH_CHARACTER);

	SearchMatch match(prefix + message->searchTerm);
	match.searchType = SearchMatch::SEARCH_FULLTEXT;
	getView()->setMatches(std::vector<SearchMatch>(1, match));
}

void SearchController::handleMessage(MessageShowErrors* message)
{
	SearchMatch match = SearchMatch::createCommand(SearchMatch::COMMAND_ERROR);
	getView()->setMatches(std::vector<SearchMatch>(1, match));
}

SearchView* SearchController::getView()
{
	return Controller::getView<SearchView>();
}

void SearchController::clear()
{
	getView()->setMatches(std::vector<SearchMatch>());
}
