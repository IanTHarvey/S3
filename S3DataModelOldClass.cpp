#include "stdafx.h"
#include "S3DataModel.h"

// ----------------------------------------------------------------------------

CS3DataModel::CS3DataModel()
{
	unsigned char i, j;

	// File version
	m_VerSW[0] = 0;
	m_VerSW[1] = 0;

	// Initialise
	for (i = 0; i < S3_MAX_MODULES; i++)
	{
		CS3ModuleData	*md = &(m_Module[i]);

		*md->m_SerialNumber = '\0';
		*md->m_ModelId = '\0';

		for (j = 0; j < S3_MAX_FOL_CHANNELS; j++)
		{
			/*
			md->m_LiveInputs[j] = false;

			CS3RxChannelData	*id = md->m_Input[j];

			id->m_Gain = 0.0;
			id->m_Impedance = 50;
			id->m_TestSig = false;
			*/
		}
	}

	strcpy_s(m_ParameterNames[0], S3_MAX_PARAMETER_LEN, "Gain");
	strcpy_s(m_ParameterNames[1], S3_MAX_PARAMETER_LEN, "Impedance");
	strcpy_s(m_ParameterNames[2], S3_MAX_PARAMETER_LEN, "Test Signal");
}

// ----------------------------------------------------------------------------

CS3DataModel::~CS3DataModel()
{
}

// ----------------------------------------------------------------------------

/*
CS3DataModel *CS3DataModel::TestSystem0(void)
{
	CS3DataModel	*dm = new CS3DataModel;
	CS3ModuleData	*md;
	CS3RxFOLData	*id;

	int				i;

	// Module 0
	md = &(dm->m_Module[0]);
	
	strcpy_s(md->m_SerialNumber, S3_MAX_SN_LEN, "0111_gh_11111");
	strcpy_s(md->m_ModelId, S3_MAX_MODEL_ID_LEN, "S3_Rec_008");
	
	/*
	for (i = 0; i < S3_MAX_TX_INPUTS; i++)
	{
		md->m_LiveInputs[i] = true;
			
		id = md->m_Input[i];

		id->m_Gain = i * 10.0;
		id->m_Impedance = 50;
		if (i < 5)
			id->m_TestSig = true;
		else
			id->m_TestSig = false;
	} */

		md = &(dm->m_Module[1]);
	/*
	strcpy_s(md->m_SerialNumber, S3_MAX_SN_LEN, "222_ef_111111");
	strcpy_s(md->m_ModelId, S3_MAX_MODEL_ID_LEN, "S3_Rec_018");


	for (i = 0; i < S3_MAX_TX_INPUTS; i++)
	{
		if (i % 3)
			md->m_LiveInputs[i] = true;
		else
			md->m_LiveInputs[i] = false;
		
		id = md->m_Input[i];

		id->m_Gain = i * 11.0;
		id->m_Impedance = 75;
		if (i < 1)
			id->m_TestSig = true;
		else
			id->m_TestSig = false;
	} */

	md = &(dm->m_Module[2]);
	strcpy_s(md->m_SerialNumber, S3_MAX_SN_LEN, "333_ef_111111");
	strcpy_s(md->m_ModelId, S3_MAX_MODEL_ID_LEN, "S3_Rec_028");

	/*
	for (i = 0; i < S3_MAX_TX_INPUTS; i++)
	{
		if (i < 1)
			md->m_LiveInputs[i] = true;
		else
			md->m_LiveInputs[i] = false;
		
		id = md->m_Input[i];

		id->m_Gain = i * 12.0;
		id->m_Impedance = 50;
		if (i < 3)
			id->m_TestSig = true;
		else
			id->m_TestSig = false;
	} */

	md = &(dm->m_Module[7]);
	strcpy_s(md->m_SerialNumber, S3_MAX_SN_LEN, "777_ef_111111");
	strcpy_s(md->m_ModelId, S3_MAX_MODEL_ID_LEN, "S3_Rec_078");

	/* for (i = 0; i < S3_MAX_TX_INPUTS; i++)
	{
		if (i < 5)
			md->m_LiveInputs[i] = true;
		else
			md->m_LiveInputs[i] = false;

		id = md->m_Input[i];

		id->m_Gain = i * 12.0;
		id->m_Impedance = 50;
		if (i % 3)
			id->m_TestSig = true;
		else
			id->m_TestSig = false;
	}
	*/

	return dm;
}
*/

// ----------------------------------------------------------------------------

S3DataModel *S3Copy(const S3DataModel *src)
{
	S3DataModel	*dest = new S3DataModel;

	return dest;
}

// ----------------------------------------------------------------------------

int S3Save(const char *Filename, const CS3DataModel *src)
{
	unsigned char	i;
	FILE	*fid;
	errno_t	err;

	err = fopen_s(&fid, Filename, "wb");

	if (err)
		return err;

	fwrite(src->m_VerSW, sizeof(unsigned int), 2, fid);

	fwrite(&src->m_NParameters, sizeof(unsigned char), 1, fid);
	fwrite(src->m_ParameterNames[0], sizeof(char), S3_MAX_PARAMETER_LEN, fid);
	fwrite(src->m_ParameterNames[1], sizeof(char), S3_MAX_PARAMETER_LEN, fid);
	fwrite(src->m_ParameterNames[2], sizeof(char), S3_MAX_PARAMETER_LEN, fid);

	for (i = 0; i < S3_MAX_MODULES; i++)
	{
		const S3ModuleData *md = &(src->m_Module[i]);
		
		fwrite(md->m_SerialNumber, sizeof(char), S3_MAX_SN_LEN, fid);
		fwrite(md->m_ModelId, sizeof(char), S3_MAX_MODEL_ID_LEN, fid);

		fwrite(md->m_Input, sizeof(S3TxInputData), S3_MAX_TX_INPUTS, fid);
	}

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------

S3DataModel *S3Read(const char *Filename)
{
	S3DataModel	*dest = new S3DataModel;

	unsigned char	i;
	FILE	*fid;
	errno_t	err;

	err = fopen_s(&fid, Filename, "rb");

	if (err)
		return NULL;

	fread_s(dest->m_VerSW, 2 * sizeof(unsigned int), sizeof(unsigned int), 2, fid);

	fread_s(&dest->m_NParameters, 1 * sizeof(unsigned char), sizeof(unsigned char), 1, fid);
	fread_s(dest->m_ParameterNames[0], S3_MAX_PARAMETER_LEN * sizeof(char), sizeof(char), S3_MAX_PARAMETER_LEN, fid);
	fread_s(dest->m_ParameterNames[1], S3_MAX_PARAMETER_LEN * sizeof(char), sizeof(char), S3_MAX_PARAMETER_LEN, fid);
	fread_s(dest->m_ParameterNames[2], S3_MAX_PARAMETER_LEN * sizeof(char), sizeof(char), S3_MAX_PARAMETER_LEN, fid);


	for (i = 0; i < S3_MAX_MODULES; i++)
	{
		const S3ModuleData *md = &(dest->m_Module[i]);

		// fread_s((void *)(md->m_LiveInputs), S3_MAX_TX_INPUTS * sizeof(bool), sizeof(bool), S3_MAX_TX_INPUTS, fid);
		fread_s((void *)(md->m_SerialNumber), S3_MAX_SN_LEN * sizeof(char), sizeof(char), S3_MAX_SN_LEN, fid);
		fread_s((void *)(md->m_ModelId), S3_MAX_MODEL_ID_LEN * sizeof(char), sizeof(char), S3_MAX_MODEL_ID_LEN, fid);

		fread_s((void *)(md->m_Input), S3_MAX_TX_INPUTS * sizeof(S3TxInputData), sizeof(S3TxInputData), S3_MAX_TX_INPUTS, fid);
	}

	fclose(fid);

	return dest;
}

// ----------------------------------------------------------------------------

int S3ParseSerialNumber(const char *sn)
{
	return 0;
}

// ----------------------------------------------------------------------------

int S3ParseModelId(const char *id)
{
	return 0;
}

// ----------------------------------------------------------------------------
