#include "DataAccessTest.h"

void DataAccessTest::createDataBase()
{
	this->_dataAccess.open();
}

void DataAccessTest::addRecords()
{
	//Create users
	User user1(1, "uesr1");
	User user2(2, "uesr2");
	User user3(3, "uesr3");

	//Create albums of users
	Album album1OfUser1(1, 1, "album1OfUser1");
	Album album2OfUser1(2, 1, "album2OfUser1");
	Album album1OfUser2(3, 2, "album1OfUser2");
	Album album2OfUser2(4, 2, "album2OfUser2");
	Album album1OfUser3(5, 3, "album1OfUser3");
	Album album2OfUser3(6, 3, "album2OfUser3");

	//Create pictures of users
	Picture pic1OfUser1(1, "pic1OfUser1", 1);
	Picture pic2OfUser1(2, "pic2OfUser1", 1);
	Picture pic1OfUser2(3, "pic1OfUser2", 3);
	Picture pic2OfUser2(4, "pic2OfUser2", 4);
	Picture pic1OfUser3(5, "pic1OfUser3", 5);
	Picture pic2OfUser3(6, "pic2OfUser3", 5);

	//Apply changes to DB
	this->_dataAccess.createUser(user1);
	this->_dataAccess.createUser(user2);
	this->_dataAccess.createUser(user3);

	this->_dataAccess.createAlbum(album1OfUser1);
	this->_dataAccess.createAlbum(album2OfUser1);
	this->_dataAccess.createAlbum(album1OfUser2);
	this->_dataAccess.createAlbum(album2OfUser2);
	this->_dataAccess.createAlbum(album1OfUser3);
	this->_dataAccess.createAlbum(album2OfUser3);

	//Add pictures to albums
	this->_dataAccess.addPictureToAlbumByName("album1OfUser1", pic1OfUser1);
	this->_dataAccess.addPictureToAlbumByName("album1OfUser1", pic2OfUser1);
	this->_dataAccess.addPictureToAlbumByName("album1OfUser2", pic1OfUser2);
	this->_dataAccess.addPictureToAlbumByName("album2OfUser2", pic2OfUser2);
	this->_dataAccess.addPictureToAlbumByName("album1OfUser3", pic1OfUser3);
	this->_dataAccess.addPictureToAlbumByName("album1OfUser3", pic2OfUser3);

	//Tag users in pictures
	this->_dataAccess.tagUserInPicture("album1OfUser1", "pic1OfUser1", 2);
	this->_dataAccess.tagUserInPicture("album1OfUser1", "pic1OfUser1", 3);
	this->_dataAccess.tagUserInPicture("album1OfUser1", "pic2OfUser1", 2);
	this->_dataAccess.tagUserInPicture("album1OfUser1", "pic2OfUser1", 3);

	this->_dataAccess.tagUserInPicture("album1OfUser2", "pic1OfUser2", 1);
	this->_dataAccess.tagUserInPicture("album1OfUser2", "pic1OfUser2", 3);
	this->_dataAccess.tagUserInPicture("album1OfUser2", "pic2OfUser2", 1);
	this->_dataAccess.tagUserInPicture("album1OfUser2", "pic2OfUser2", 3);

	this->_dataAccess.tagUserInPicture("album1OfUser3", "pic1OfUser3", 1);
	this->_dataAccess.tagUserInPicture("album1OfUser3", "pic1OfUser3", 2);
	this->_dataAccess.tagUserInPicture("album1OfUser3", "pic2OfUser3", 1);
	this->_dataAccess.tagUserInPicture("album1OfUser3", "pic2OfUser3", 2);

	this->_dataAccess.deleteUser(user2);
}