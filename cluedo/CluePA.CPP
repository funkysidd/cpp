#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#define NUM_WHOS   6
#define NUM_WHATS  6
#define NUM_WHERES 9

template <std::size_t SIZE>
using GenericNameArray = std::array<std::string, SIZE>;

GenericNameArray<NUM_WHOS> _whoList{"green",
                                    "mustard",
                                    "peacock",
                                    "plum",
                                    "scarlet",
                                    "white"};

GenericNameArray<NUM_WHATS> _whatList{"wrench",
                                      "candlestick",
                                      "dagger",
                                      "pistol",
                                      "lead pipe",
                                      "rope"};

GenericNameArray<NUM_WHERES> _whereList{"bathroom",
                                        "office",
                                        "dining room",
                                        "game room",
                                        "garage",
                                        "bedroom",
                                        "living room",
                                        "kitchen",
                                        "courtyard"};

struct GenericCardObject
{
  GenericCardObject(std::string name, unsigned int numPlayers)
  {
    _name = name;
    _numPlayers = numPlayers;
    _taken= false;
    _playerMayHave.resize(_numPlayers, true); // All players may have the card
  }

  bool operator()(GenericCardObject const &a,
                  GenericCardObject const &b)
  {
    return (a._name < b._name);
  }

  bool getTaken() { return _taken; }
  void setTaken(bool taken) { _taken = taken; }

  std::string _name;
  unsigned int _numPlayers;
  bool _taken;
  std::vector<bool> _playerMayHave;
};

using GenericCardMap = std::map<std::string, std::unique_ptr<GenericCardObject>>;
GenericCardMap _whoCards;
GenericCardMap _whatCards;
GenericCardMap _whereCards;

// Maps user names to indices
std::map<std::string, unsigned int> _userNames;

GenericCardObject* hasCard(std::string const &cardName)
{
  auto it = _whoCards.find(cardName);
  if (it != _whoCards.end())
  {
    return it->second.get();
  }

  it = _whatCards.find(cardName);
  if (it != _whatCards.end())
  {
    return it->second.get();
  }

  it = _whereCards.find(cardName);
  if (it != _whereCards.end())
  {
    return it->second.get();
  }

  return nullptr;
}

bool hasUser(std::string const &userName)
{
  return (_userNames.find(userName) != _userNames.end());
}

unsigned int getNumberOfUsers()
{
  return _userNames.size();
}

template <std::size_t SIZE>
void initCardArray(GenericCardMap &cardMap,
                   GenericNameArray<SIZE> const &nameArray,
                   unsigned int numPlayers)
{
  for (auto const &name: nameArray)
  {
    cardMap[name] = std::unique_ptr<GenericCardObject>(new GenericCardObject(name, numPlayers));
  }
}

void toLower(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void markCardAsTaken(std::string const &cardName)
{
  if (auto *card = hasCard(cardName))
  {
    card->setTaken(true);
  }
  else
  {
    std::cout << "Card name is invalid!" << std::endl;
  }
}

void parseCardNames(std::string const &commaSepCardNames,
                    std::vector<std::string> &cardNames)
{
  cardNames.clear();
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char> sep(",");
  tokenizer tok(commaSepCardNames, sep);
  for(auto const & token : tok)
  {
    std::string trimmedStr(token);
    boost::trim(trimmedStr);
    cardNames.push_back(trimmedStr);
  }
}

void getValidCardNames(std::vector<std::string> const &ipCardNames,
                       std::vector<std::string> &opCardNames)
{
  opCardNames.clear();
  for (auto const &cardName : ipCardNames) {
    if (!hasCard(cardName)) {
      std::cout << "Card name doesn't exists: " << cardName << std::endl;
    }
    else {
      opCardNames.push_back(cardName);
    }
  }
}

bool updateUserInputInternal(GenericCardMap &cardMap,
                             std::string const &cardName,
                             unsigned int userIndex)
{
  auto it = cardMap.find(cardName);
  if (it != cardMap.end())
  {
    auto &cardObject = *(it->second.get());
    if (!cardObject.getTaken())
    {
      cardObject._playerMayHave[userIndex] = false; // Player doesn't have the card
      return true;
    }
  }

  return false;
}

void updateUserInput(std::string const &userName,
                     std::vector<std::string> const &cardNames)
{
  auto it = _userNames.find(userName);
  if (it == _userNames.end()) {
    std::cout << "Invalid user name!" << std::endl;
    return;
  }

  unsigned int userIndex = it->second;
  std::cout << "User index (" << userName << "): " << userIndex << std::endl;
  for (auto const &cardName : cardNames)
  {
    bool success = updateUserInputInternal(_whoCards, cardName, userIndex);
    if (!success) {
      success = updateUserInputInternal(_whatCards, cardName, userIndex);
    }
    if (!success) {
      success = updateUserInputInternal(_whereCards, cardName, userIndex);
    }
  }
}

void updateTakenCards(std::vector<std::string> const &cardNames)
{
  // Mark the third card as taken if the other two have already been taken
  std::set<GenericCardObject *> nonTakenCards;
  for (auto const &cardName : cardNames)
  {
    if (auto *card = hasCard(cardName)) {
      if (!card->getTaken()) {
        nonTakenCards.insert(card);
      }
    }
  }

  if (nonTakenCards.size() == 1) { // Ideal case
    auto it = nonTakenCards.begin();
    (*it)->setTaken(true);
  }
  // else if (nonTakenCards.size() == 0) {
  //   std::cout << "All input cards are taken, user may be lying!" << std::endl;
  // }
}

void testForClueInternal(GenericCardMap const &cardMap,
                         std::string const &mapName)
{
  for (auto it = cardMap.begin(); it != cardMap.end(); ++it)
  {
    bool result = false;
    auto *card = it->second.get();
    auto const &playerMayHave = card->_playerMayHave;
    for (int i = 0; i < playerMayHave.size(); ++i) {
      result |= playerMayHave[i];
    }

    if (!result) {
      std::cout << "Missing card from the list (" << mapName << "): "
                << card->_name << std::endl;
    }
  }
}

void testForClue()
{
  testForClueInternal(_whoCards, "who");
  testForClueInternal(_whatCards, "what");
  testForClueInternal(_whereCards, "where");
}

void getUserNames()
{
  std::cout << "!!!Fetching user names!!!" << std::endl;
  unsigned char userInput = 'n';
  do {
    std::string userName;
    std::cout << "Enter a user name: ";
    std::cin >> userName;
    if (hasUser(userName)) {
      std::cout << "User name already exists!" << std::endl;
    }
    else {
      _userNames[userName] = _userNames.size();
      std::cout << "User name successfully added!" << std::endl;
    }
    std::cout << "Continue? (y/n): ";
    std::cin >> userInput;
  } while (userInput != 'n');
  std::cout << "!!!Done!!!" << std::endl;
}

void initPreGameState()
{
  std::cout << "!!!Initializing pre-game state!!!" << std::endl;
  unsigned char userInput = 'n';
  do {
    std::string cardName;
    std::cout << "Enter a card name: ";
    std::cin.ignore();
    std::getline(std::cin, cardName);
    toLower(cardName);
    markCardAsTaken(cardName);
    std::cout << "Continue? (y/n): ";
    std::cin >> userInput;
  } while (userInput != 'n');
  std::cout << "!!!Done initializing!!!" << std::endl;
}

void mainGameLoop()
{
  std::cout << "!!!Starting main game loop!!!" << std::endl;
  unsigned char userInput = 'n';
  do {
    std::string userName, commaSepCardNames;
    std::vector<std::string> ipCardNames;
    std::vector<std::string> opCardNames;

    bool success = false;
    do {
      std::cout << "Enter user name: ";
      std::cin >> userName;
      success = hasUser(userName);
      if (!success) { std::cout << "  <<<User name invalid, please enter again!>>>"
                                << std::endl; }
    } while (!success);

    success = false;
    bool ignoreOnce = true;
    do {
      std::cout << "Enter card trio (comma separated): ";
      if (ignoreOnce) {
        std::cin.ignore();
        ignoreOnce = false;
      }
      std::getline(std::cin, commaSepCardNames);
      parseCardNames(commaSepCardNames, ipCardNames);
      getValidCardNames(ipCardNames, opCardNames);
      success = (opCardNames.size() == 3);
      if (!success) { std::cout << "  <<<Card trio not valid, please enter again!>>>"
                                << std::endl; }
    } while (!success);

    updateUserInput(userName, opCardNames);

    std::cout << "Do any of the three cards exist? (y/n): ";
    std::cin >> userInput;
    if (userInput == 'y') {
      updateTakenCards(opCardNames);
    }

    testForClue();

    std::cout << "Continue? (y/n): ";
    std::cin >> userInput;
  } while (userInput != 'n');
  std::cout << "!!!End main game loop!!!" << std::endl;
}

int main()
{
  getUserNames();

  initCardArray(_whoCards, _whoList, getNumberOfUsers());
  initCardArray(_whatCards, _whatList, getNumberOfUsers());
  initCardArray(_whereCards, _whereList, getNumberOfUsers());

  initPreGameState();

  mainGameLoop();

  return 0;
}
