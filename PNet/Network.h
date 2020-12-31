#pragma once
#include <WinSock2.h>
#include <iostream>


namespace PNet {

	class Network
	{
	public:
		static bool Init();
		static void Kill();
	};

}