#pragma once
#include <io.h>
#include <algorithm>
#include <string>
#include "IDataAccess.h"
#include "ItemNotFoundException.h"
#include "sqlite3.h"

using std::string;
using std::pair;

class DatabaseAccess : public IDataAccess
{
public:
	DatabaseAccess() = default; //constructor
	virtual ~DatabaseAccess() = default; //destructor

	// album related
	int getNextAlbumId(); //return next album id
	const std::list<Album> getAlbums() override; //return _albums
	const std::list<Album> getAlbumsOfUser(const User& user) override; //return albums of user
	void createAlbum(const Album& album) override; //create album
	void deleteAlbum(const std::string& albumName, int userId) override; //delete album
	bool doesAlbumExists(const std::string& albumName, int userId) override; //check if album exists
	Album openAlbum(const std::string& albumName) override { return Album(); }; //override but not defined
	Album openAlbum(const std::string& albumName, const int userId); //open album using album name and user id
	void closeAlbum(Album &pAlbum) override; //close album
	void printAlbums() override; //print albums

	// picture related
	int getNextPictureId(); //return next picture id
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override; //add picture to album
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override; //remove picture from album
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; //tag user in picture
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; //untag user in picture

	// user related
	int getNextUserId() const; //return next user id
	const std::list<User> getUsers() const; //return _users
	void printUsers() override; //print users
	void createUser(User& user) override; //create user
	void deleteUser(const User& user) override; //delete user
	bool doesUserExists(int userId) override; //check if user exists
	User getUser(int userId) override; //return User

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override; //return number of owned albums of user
	int countAlbumsTaggedOfUser(const User& user) override; //return number of albums the user is tagged in
	int countTagsOfUser(const User& user) override; //return in how many pictures the user is tagged
	float averageTagsPerAlbumOfUser(const User& user) override; //return countTagsOfUser()/countAlbumsTaggedOfUser()

	// Queries
	User getTopTaggedUser() override; //return top tagged user
	Picture getTopTaggedPicture() override; //return top tagged picture
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override; //return list of pictures the user is tagged in them

	bool open() override; //open DB
	void close() override; //close DB
	void clear() override; //clear class objects

private:
	sqlite3* _DB;
	std::list<Album> _albums;
	std::list<User> _users;

	auto getAlbumIfExists(const std::string& albumName); //return Album if exists
	Picture getPictureByName(string name); //return Picture by its name

	//callback functions
	static int topUserCallback(void* data, int argc, char** argv, char** colName); //get top tagged user
	static int topPicCallback(void* data, int argc, char** argv, char** colName); //get top tagged picture
};