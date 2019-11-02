#include "DatabaseAccess.h"
#include <vector>
#include <algorithm>

sqlite3* DB; //Global DB... Noooo, I'm sorry but with this program design that's all I could do.

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::to_string;

const std::list<User> getUsersFromDB();
const std::list<Album> getAlbumsFromDB();
const std::list<Picture> getPicturesFromDB();
const std::list<pair<int, int>> getTagsFromDB();

int usersCallback(void* data, int argc, char** argv, char** colName);
int albumsCallback(void* data, int argc, char** argv, char** colName);
int picturesCallback(void* data, int argc, char** argv, char** colName);
int tagsCallback(void* data, int argc, char** argv, char** colName);

//private functions
auto DatabaseAccess::getAlbumIfExists(const std::string& albumName)
{
	auto result = std::find_if(std::begin(this->_albums), std::end(this->_albums), [&](auto& album) { return album.getName() == albumName; });

	if (result == std::end(this->_albums))
	{
		throw ItemNotFoundException("Album not exists: ", albumName);
	}
	return result;
}

Picture DatabaseAccess::getPictureByName(string name)
{
	std::list<Picture> pictures = getPicturesFromDB();

	for (const auto& pic : pictures)
	{
		if (pic.getName() == name)
		{
			return pic;
		}
	}
	return Picture(0, "", 0);
}

int DatabaseAccess::topUserCallback(void* data, int argc, char** argv, char** colName)
{
	int userId = argc > 0 ? atoi(argv[0]) : 0;
	std::list<User> users = getUsersFromDB();

	for (const auto& user : users)
	{
		if (user.getId() == userId)
		{
			((std::list<User>*)data)->push_back(user);
			break;
		}
	}

	return 0;
}

int DatabaseAccess::topPicCallback(void* data, int argc, char** argv, char** colName)
{
	int picId = argc > 0 ? atoi(argv[0]) : 0;
	std::list<Picture> pictures = getPicturesFromDB();

	for (const auto& pic : pictures)
	{
		if (pic.getId() == picId)
		{
			((std::list<Picture>*)data)->push_back(pic);
			break;
		}
	}

	return 0;
}

//public functions
bool DatabaseAccess::open()
{
	string dbFileName = "GalleryDB.sqlite";
	int fileExists = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &this->_DB);

	if (res != SQLITE_OK)
	{
		this->_DB = nullptr;
		cerr << "Failed to open " << dbFileName << endl;
		return false;
	}

	if (fileExists == -1) //file doesn't exist
	{
		const char* sqlQuery = "CREATE TABLE IF NOT EXISTS USERS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT UNIQUE NOT NULL);"
			"CREATE TABLE IF NOT EXISTS ALBUMS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(USER_ID) REFERENCES USERS(ID));"
			"CREATE TABLE IF NOT EXISTS PICTURES(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));"
			"CREATE TABLE IF NOT EXISTS TAGS(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID), FOREIGN KEY(USER_ID) REFERENCES USERS(ID));";
		char** errMessage = nullptr;
		sqlite3_exec(this->_DB, sqlQuery, nullptr, nullptr, errMessage);
	}

	DB = this->_DB;
	this->_albums = getAlbumsFromDB();
	this->_users = getUsersFromDB();
	return true;
}

void DatabaseAccess::close()
{
	sqlite3_close(this->_DB);
	DB = nullptr;
	this->_DB = nullptr;
}

void DatabaseAccess::clear()
{
	this->_albums.clear();
	this->_users.clear();
}

int DatabaseAccess::getNextAlbumId()
{
	std::list<Album> albums = getAlbums();
	vector<int> albumsIds;

	for (const auto& album : albums)
	{
		albumsIds.push_back(album.getId());
	}
	std::sort(albumsIds.begin(), albumsIds.end());

	return albumsIds.size() > 0 ? albumsIds.back() : 0;
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	return this->_albums;
}

int DatabaseAccess::getNextUserId() const
{
	std::list<User> users = getUsers();
	vector<int> usersIds;

	for (const auto& user : users)
	{
		usersIds.push_back(user.getId());
	}
	std::sort(usersIds.begin(), usersIds.end());

	return usersIds.size() > 0 ? usersIds.back() : 0;
}

const std::list<User> DatabaseAccess::getUsers() const
{
	return this->_users;
}

int DatabaseAccess::getNextPictureId()
{
	std::list<Picture> pictures = getPicturesFromDB();
	vector<int> picturesIds;

	for (const auto& pic : pictures)
	{
		picturesIds.push_back(pic.getId());
	}
	std::sort(picturesIds.begin(), picturesIds.end());

	return picturesIds.size() > 0 ? picturesIds.back() : 0;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> albums = getAlbums();
	std::list<Album> albumsOfUser;

	for (auto it = albums.begin(); it != albums.end(); it++)
	{
		if (it->getOwnerId() == user.getId())
		{
			albumsOfUser.push_back(*it);
		}
	}

	return albumsOfUser;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	string sqlQuery = "INSERT INTO ALBUMS VALUES(" + to_string(album.getId()) + ", '" + album.getName() + "', '" + album.getCreationDate() + "', " + to_string(album.getOwnerId()) + ");";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);
	this->_albums.push_back(album);
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	std::list<Album> albums = getAlbums();
	std::list<Picture> pictures;

	for (const auto& album : albums)
	{
		if (album.getName() == albumName && album.getOwnerId() == userId)
		{
			pictures = album.getPictures();
			break;
		}
	}

	for (const auto& pic : pictures)
	{
		removePictureFromAlbumByName(albumName, pic.getName());
	}
	
	string sqlQuery = "DELETE FROM ALBUMS WHERE NAME = '" + albumName + "' AND USER_ID = " + to_string(userId) + ";";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	for (auto it = this->_albums.begin(); it != this->_albums.end(); it++)
	{
		if (it->getName() == albumName && it->getOwnerId() == userId) 
		{
			it = this->_albums.erase(it);
			break;
		}
	}
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	bool exists = false;
	std::list<Album> albums = getAlbums();

	for (auto it = albums.begin(); it != albums.end(); it++)
	{
		if (it->getName() == albumName && it->getOwnerId() == userId)
		{
			exists = true;
		}
	}

	return exists;
}

Album DatabaseAccess::openAlbum(const std::string& albumName, const int userId)
{
	std::list<Album> albums = getAlbums();

	for (auto& album : albums)
	{
		if (albumName == album.getName() && album.getOwnerId() == userId) 
		{
			return album;
		}
	}
	throw MyException("No album with name " + albumName + " exists");
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{

}

void DatabaseAccess::printAlbums()
{
	std::list<Album> albums = getAlbums();

	if (albums.empty())
	{
		throw MyException("There are no existing albums.");
	}
	cout << "Album list:" << endl;
	cout << "-----------" << endl;
	
	for (const Album& album : albums) 	
	{
		cout << std::setw(5) << "* [" << album.getName() << "] - created by user@" << album.getOwnerId() << " - " << album.getCreationDate() << endl;
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	string sqlQuery = "INSERT INTO PICTURES VALUES(" + to_string(picture.getId()) + ", '" + picture.getName() + "', '" + picture.getPath() + "', '" + picture.getCreationDate() + "', " + to_string(picture.getAlbumId()) + ");";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	auto result = getAlbumIfExists(albumName);
	(*result).addPicture(picture);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	string sqlQuery = "DELETE FROM TAGS WHERE PICTURE_ID = " + to_string(getPictureByName(pictureName).getId()) + ";";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);
	sqlQuery = "DELETE FROM PICTURES WHERE NAME = '" + pictureName + "' AND ALBUM_ID = " + to_string(getPictureByName(pictureName).getAlbumId()) + ";";
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	auto result = getAlbumIfExists(albumName);
	(*result).removePicture(pictureName);
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	string sqlQuery = "INSERT INTO TAGS VALUES(NULL, " + to_string(getPictureByName(pictureName).getId()) + ", " + to_string(userId) + ");";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	auto result = getAlbumIfExists(albumName);
	(*result).tagUserInPicture(userId, pictureName);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	string sqlQuery = "DELETE FROM TAGS WHERE PICTURE_ID = " + to_string(getPictureByName(pictureName).getId()) + " AND USER_ID = " + to_string(userId) + ";";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	auto result = getAlbumIfExists(albumName);
	(*result).untagUserInPicture(userId, pictureName);
}

void DatabaseAccess::printUsers()
{
	std::list<User> users = getUsers();

	std::cout << "Users list:" << std::endl;
	std::cout << "-----------" << std::endl;
	
	for (const auto& user: users)
	{
		cout << user << endl;
	}
}

void DatabaseAccess::createUser(User& user)
{
	string sqlQuery = "INSERT INTO USERS VALUES(" + to_string(user.getId()) + ", '" + user.getName() + "');";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);
	this->_users.push_back(user);
}

void DatabaseAccess::deleteUser(const User& user)
{
	std::list<Album> albumsOfUser = getAlbumsOfUser(user);

	for (const auto& album : albumsOfUser)
	{
		deleteAlbum(album.getName(), user.getId());
	}

	string sqlQuery = "DELETE FROM TAGS WHERE USER_ID = " + to_string(user.getId()) + ";";
	char** errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	sqlQuery = "DELETE FROM USERS WHERE ID = " + to_string(user.getId()) + ";";
	errMessage = nullptr;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), nullptr, nullptr, errMessage);

	if (doesUserExists(user.getId()))
	{
		for (auto iter = this->_users.begin(); iter != this->_users.end(); iter++)
		{
			if (*iter == user)
			{
				for (auto it = this->_albums.begin(); it != this->_albums.end(); it++)
				{
					it->untagUserInAlbum(iter->getId());
	
					if (it->getOwnerId() == iter->getId())
					{
						it = this->_albums.erase(it);
						
						if (it == this->_albums.end())
						{
							break;
						}
					}
				}
				iter = this->_users.erase(iter);
				break;
			}
		}
	}
}

bool DatabaseAccess::doesUserExists(int userId)
{
	bool exists = false;
	std::list<User> users = getUsers();

	for (auto it = users.begin(); it != users.end(); it++)
	{
		if (it->getId() == userId)
		{
			exists = true;
		}
	}

	return exists;
}

User DatabaseAccess::getUser(int userId)
{
	std::list<User> users = getUsers();

	for (const auto& user : users) 
	{
		if (user.getId() == userId) 
		{
			return user;
		}
	}
	throw ItemNotFoundException("User", userId);
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	std::list<Album> albums = getAlbums();
	int count = 0;
	
	for (const auto& album: albums) 
	{
		if (album.getOwnerId() == user.getId()) 
		{
			count++;
		}
	}
	
	return count;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int count = 0;
	
	for (const auto& album: this->_albums)
	{
		const std::list<Picture>& pics = album.getPictures();
		
		for (const auto& picture: pics)
		{
			if (picture.isUserTagged(user))
			{
				count++;
				break;
			}
		}
	}
	
	return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	std::list<Album> albums = getAlbumsFromDB();
	std::list<Picture> pictures;
	int count = 0;
	
	for (const auto& album : albums)
	{
		for (const auto& pic : album.getPictures())
		{
			pictures.push_back(pic);
		}
	}

	for (const auto& pic : pictures)
	{
		if (pic.isUserTagged(user.getId()))
		{
			count++;
		}
	}
	
	return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albumsTaggedCount = countAlbumsTaggedOfUser(user);
	
	if (albumsTaggedCount == 0)
	{
		return 0;
	}
	return static_cast<float>(countTagsOfUser(user)) / albumsTaggedCount;
}

User DatabaseAccess::getTopTaggedUser()
{
	string sqlQuery = "SELECT USER_ID FROM TAGS GROUP BY USER_ID ORDER BY COUNT(*) DESC LIMIT 1;";
	char** errMessage = nullptr;
	std::list<User> topUser;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), topUserCallback, &topUser, errMessage);

	if (topUser.size() == 0)
	{
		throw MyException("There isn't any tagged user.");
	}

	return topUser.front(); //if there isn't a user that is tagged more than other users then the function will return the first user in the database.
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	string sqlQuery = "SELECT PICTURE_ID FROM TAGS GROUP BY PICTURE_ID ORDER BY COUNT (*) DESC LIMIT 1;";
	char** errMessage = nullptr;
	std::list<Picture> topPic;
	sqlite3_exec(this->_DB, sqlQuery.c_str(), topPicCallback, &topPic, errMessage);

	if (topPic.size() == 0)
	{
		throw MyException("There isn't any tagged picture.");
	}
	
	return topPic.front(); //same as for top tagged user
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::list<Album> albums = getAlbumsFromDB();
	std::list<Picture> pictures;
	std::list<Picture> userPictures;

	for (const auto& album : albums)
	{
		for (const auto& pic : album.getPictures())
		{
			pictures.push_back(pic);
		}
	}

	for (const auto& pic : pictures)
	{
		if (pic.isUserTagged(user.getId()))
		{
			userPictures.push_back(pic);
		}
	}
	
	return userPictures;
}

//Helper functions, outside of class DatabaseAccess
const std::list<User> getUsersFromDB()
{
	std::list<User> users;
	string sqlQuery = "SELECT * FROM USERS;";
	char** errMessage = nullptr;
	sqlite3_exec(DB, sqlQuery.c_str(), usersCallback, &users, errMessage);

	return users;
}

const std::list<Album> getAlbumsFromDB()
{
	std::list<Album> albums;
	std::list<Picture> pictures = getPicturesFromDB();
	std::list<pair<int, int>> tags = getTagsFromDB();
	string sqlQuery = "SELECT * FROM ALBUMS;";
	char** errMessage = nullptr;
	sqlite3_exec(DB, sqlQuery.c_str(), albumsCallback, &albums, errMessage);

	for (auto& album : albums)
	{
		for (const auto& pic : pictures)
		{
			if (album.getId() == pic.getAlbumId())
			{
				album.addPicture(pic);
			}

			for (const auto& tag : tags)
			{
				if (tag.first == pic.getId())
				{
					album.tagUserInPicture(tag.second, pic.getName());
				}
			}
		}
	}

	return albums;
}

const std::list<Picture> getPicturesFromDB()
{
	std::list<Picture> pictures;
	string sqlQuery = "SELECT * FROM PICTURES;";
	char** errMessage = nullptr;
	sqlite3_exec(DB, sqlQuery.c_str(), picturesCallback, &pictures, errMessage);

	return pictures;
}

const std::list<pair<int, int>> getTagsFromDB()
{
	std::list<pair<int, int>> tags;
	string sqlQuery = "SELECT * FROM TAGS;";
	char** errMessage = nullptr;
	sqlite3_exec(DB, sqlQuery.c_str(), tagsCallback, &tags, errMessage);

	return tags;
}

int usersCallback(void* data, int argc, char** argv, char** colName)
{
	User user(atoi(argv[0]), argv[1]);
	((std::list<User>*)data)->push_back(user);

	return 0;
}

int albumsCallback(void* data, int argc, char** argv, char** colName)
{
	Album album(atoi(argv[0]), atoi(argv[3]), argv[1], argv[2]);
	((std::list<Album>*)data)->push_back(album);

	return 0;
}

int picturesCallback(void* data, int argc, char** argv, char** colName)
{
	Picture picture(atoi(argv[0]), argv[1], argv[2], argv[3], atoi(argv[4]));
	((std::list<Picture>*)data)->push_back(picture);

	return 0;
}

int tagsCallback(void* data, int argc, char** argv, char** colName)
{
	pair<int, int> tag(atoi(argv[1]), atoi(argv[2]));
	((std::list<pair<int, int>>*)data)->push_back(tag);

	return 0;
}