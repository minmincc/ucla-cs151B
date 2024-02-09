#include "cache.h"

cache::cache()
{
	for (int i = 0; i < L1_CACHE_SETS; i++)
		L1[i].valid = false;
	for (int i = 0; i < L2_CACHE_SETS; i++)
		for (int j = 0; j < L2_CACHE_WAYS; j++)
		{
			L2[i][j].valid = false;
			L2[i][j].lru_position = -1;
		}
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		Vict[i].valid = false;
		Vict[i].lru_position = -1;
	}
	this->myStat.missL1 = 0;
	this->myStat.missL2 = 0;
	this->myStat.accVic = 0;
	this->myStat.missVic = 0;
	this->myStat.accL1 = 0;
	this->myStat.accL2 = 0;
}
//this is to look for the address in L1
bool cache::lookForL1(string binaryAddress)
{
	//make 2 bit for block offset, next 4 bits for index, next 26 bits for tag
	string block_offset = binaryAddress.substr(30, 2);   
	string Line_index = binaryAddress.substr(26, 4);    
	string string_tag = binaryAddress.substr(0, 26);      
	int index =std::stoi(Line_index, nullptr, 2);
	int tag_index = std::stoi(string_tag, nullptr, 2);
	return L1[index].valid && (L1[index].tag == tag_index);
}

int cache::lookForVict(string real_adr)
{
	//just make 2 bits for offset and rest for the tag to search
	string string_tag = real_adr.substr(0, 30); 
	int tag_index = std::stoi(string_tag, nullptr, 2);
	
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		if (Vict[i].valid && (Vict[i].tag == tag_index))
		{
			return i;
		}
	}
	return -1;
}

//look for L2 cache for the address 
int cache::lookForL2(string real_adr)
{
	
	//same as L1, 2 bits for block offset, next 4 bits for index, and 26 bits for tag
	string block_offset = real_adr.substr(30, 2);   
	string Line_index = real_adr.substr(26, 4);    
	string string_tag = real_adr.substr(0, 26); 
	int index = std::stoi(Line_index, nullptr, 2);
	
	int tag_index =std::stoi(string_tag, nullptr, 2);
	
	for (int i = 0; i < 8; i++)
	{
		if (L2[index][i].valid && (L2[index][i].tag == tag_index))
		{
			//return the # of way of the address if we can find one
			return i;
		}
	}
	
	return -1;
}


//bring the most recented access address to L1
void cache::BringtoL1(string binaryAddress, int newData)
{
	//get the 2 bits offset, 4 bits index, 26 bits tag
	std::string blockOffsetStr = binaryAddress.substr(30, 2);
    int block_offset = std::stoi(blockOffsetStr, nullptr, 2);
    
    std::string lineIndexStr = binaryAddress.substr(26, 4);
    int line_index = std::stoi(lineIndexStr, nullptr, 2);
    
    std::string tagIndexStr = binaryAddress.substr(0, 26);
    int tag_index = std::stoi(tagIndexStr, nullptr, 2);
	//check if the index line is empty,we filled the data and tag
	if (!L1[line_index].valid)
	{
		
		L1[line_index].data = newData;
		L1[line_index].tag = tag_index;
		L1[line_index].valid = true;
	}
	//otherwise, we need to kick out the old data to victim and update the new L1 index line 
	else
	{
		//store the dat we will kick out
		int temp_data = L1[line_index].data;
		int temp_tag = L1[line_index].tag;

		//store the new data to the L1
		L1[line_index].data = newData;
		L1[line_index].tag = tag_index;

		//get the original address through the old tag with the index
		string tag_value = bitset<32>(temp_tag).to_string();
		string victag = tag_value.substr(6, 26);
		string index_value = bitset<32>(line_index).to_string();
		
		//what we get will be the tag for victim cache
		string str_vicindex = index_value.substr(28, 4);
		
		string new_vicadr = victag + str_vicindex;
		
		//convert to int
		int vic_new_tag = std::stoi(new_vicadr, nullptr, 2);

		//pass in to victim
		BringtoVict(vic_new_tag, temp_data);
	}
}

//bring to Vict for the data kicked from L1
void cache::BringtoVict(int newtag, int newData)
{
	//search any available space
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		if (!Vict[i].valid)
		{
			//store new data if it has it. 
			Vict[i].tag = newtag;
			Vict[i].data = newData;
			Vict[i].valid = true;
			//update the new data with the highest Lru position
			Vict[i].lru_position = 3;
			for (int i = 0; i < 4; i++)
			{
				//any other old data should decrease the lru position. 
				if ((Vict[i].lru_position != -1) && (Vict[i].lru_position != 0))
				{
					
					Vict[i].lru_position--;
				}
			}
			
			return;
		}
	}
	//if the victim cache is fulled, we need to kicked the oldest data and update the new data 
	//and also move to L2
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		if (Vict[i].lru_position == 0)
		{
			int temp_tag = Vict[i].tag;
			int temp_data = Vict[i].data;
			Vict[i].tag = newtag;
			Vict[i].data = newData;
			Vict[i].valid = true;
			//update the new data with highest lru position.
			Vict[i].lru_position = 3;
			for (int j = 0; j < VICTIM_SIZE; j++)
			{
				if ((Vict[j].lru_position != -1) && (Vict[j].lru_position != 0))
				{
					
					Vict[j].lru_position--;
				}
			}
			//get the original address from the old data's tag
			string str_adr = bitset<32>(temp_tag).to_string(); 
			string Line_index = str_adr.substr(28, 4);			
			string tag_index = str_adr.substr(2, 26);			
			
			int New_index = std::stoi(Line_index, nullptr, 2);
			int New_tag = std::stoi(tag_index, nullptr, 2);
			
			BringToL2( New_index, New_tag, temp_data);

			return;
		}
	}
}

//to see if any available space for store data in L2
int cache::findWayOFL2(int index)
{
	for (int i = 0; i < 8; i++)
	{
		if (L2[index][i].lru_position == -1)
		{

			return i;
		}
		
		if (L2[index][i].lru_position == 0)
		{
			return i;
		}
	}
	return 0;
}

//write to L2 
void cache::BringToL2(int index, int newTag, int newData)
{
	int way = findWayOFL2(index); 
	
	L2[index][way].tag = newTag;
	L2[index][way].data = newData;
	L2[index][way].valid = true;
	
	//update the lru position when write in new data
	for (int i = 0; i < 8; i++)
	{
		
		if ((L2[index][i].lru_position != -1) && (L2[index][i].lru_position != 0))
		{
			
			L2[index][i].lru_position--;
		}
	}
	//make the new data will highest lru position.
	L2[index][way].lru_position = 7;
}



bool cache::controller(bool MemR, bool MemW, int *data, int adr, int *myMem)
{
	//make the address int to 32 bits binary number
    std::string binaryAddress = std::bitset<32>(adr).to_string();
    
	//2 bits for offset
    std::string blockOffsetStr = binaryAddress.substr(30, 2);
    int block_offset = std::stoi(blockOffsetStr, nullptr, 2);
    
	//next 4 bits for index 
    std::string lineIndexStr = binaryAddress.substr(26, 4);
    int line_index = std::stoi(lineIndexStr, nullptr, 2);
    //next 26 bits for index

    std::string tagIndexStr = binaryAddress.substr(0, 26);
    int tag_index = std::stoi(tagIndexStr, nullptr, 2);

	//check if it is lw instruction
	if (MemR) {
        if (lookForL1(binaryAddress)) {
            //if in L1, we stop, but add access L1
            this->myStat.accL1++;
        } 
		else if(lookForVict(binaryAddress)!=-1){
            //if in vitcim, we stop and bring to L1, update the lru position for next of the data
            this->myStat.accL1++;
            this->myStat.missL1++;
			this->myStat.accVic++;
			int isFound_vict = lookForVict(binaryAddress);
			Vict[isFound_vict].valid = false;
			int old_position = Vict[isFound_vict].lru_position;
			for (int i = 0; i < 4; i++)
			{ 
				if ((Vict[i].lru_position < old_position) && (Vict[i].lru_position != -1))
				{
					Vict[i].lru_position++;
				}
			}
			Vict[isFound_vict].lru_position = -1;
			BringtoL1(binaryAddress,Vict[isFound_vict].data);
        }
		else if(lookForL2(binaryAddress)!=-1)
		{
			//if found in L2, we stop, write to L1 and update the lru position for rest of the data
				this->myStat.accL1++;
				this->myStat.accL2++;
				this->myStat.missL1++;
				this->myStat.missVic++;
				this->myStat.accVic++;
				int isfound_L2 = lookForL2(binaryAddress);
				L2[line_index][isfound_L2].valid = false;
				int old_position = L2[line_index][isfound_L2].lru_position;
				for (int i = 0; i < 8; i++)
				{
					if ((L2[line_index][i].lru_position < old_position) && (L2[line_index][i].lru_position != -1))
					{
						L2[line_index][i].lru_position++;
					}
				}
				L2[line_index][isfound_L2].lru_position = -1;
				BringtoL1(binaryAddress,myMem[adr]);
		}
		else
		{
			//if not found in L1, victim, L2, we should go to main memory,
			//add all acccess and misses, write the data to L1.
				this->myStat.accL1++;
				this->myStat.accL2++;
				this->myStat.missL1++;
				this->myStat.missL2++;
				this->myStat.accVic++;
				this->myStat.missVic++;
				BringtoL1(binaryAddress,myMem[adr]);
		}
    }
	// if it is sw, since it is write no allocate, we do not bring any data to cache from main memory
	if(MemW)
	{
		myMem[adr] = *data;
	}
	return true;
}

