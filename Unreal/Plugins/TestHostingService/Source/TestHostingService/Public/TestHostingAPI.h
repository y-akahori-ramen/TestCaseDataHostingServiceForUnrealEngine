// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

namespace TestHosting
{
	
	/**
	 * @brief API���g�p����̂ɕK�v�Ȋe����
	 */
	class TESTHOSTINGSERVICE_API FContext final
	{
	public:
		/**
		 * @brief �R���X�g���N�^
		 * @param User ���[�U�[��
		 * @param Password �p�X���[�h
		 * @param ServiceURI �e�X�g�z�X�e�B���O�T�[�r�X��URI
		 */
		FContext(const FString& User, const FString& Password, const FString& ServiceURI);

		/**
		 * @brief �e�X�g�P�[�X�擾�p��URI���擾����
		 * @param TestCaseDataName �擾�������e�X�g�P�[�X��
		 */
		FString GetURIGetTestCaseData(FString TestCaseDataName) const;

		/**
		 * @brief �e�X�g�P�[�X�ǉ��p��URI���擾����
		 */
		FString GetURIAddTestCaseData() const;

		/**
		 * @brief �e�X�g�P�[�X���ꗗ�擾�p��URI���擾����
		 */
		FString GetURIGetTestCaseNameList() const;
		
		/**
		 * @brief HTTP���N�G�X�g��Basic�F�؃w�b�_�[��ǉ�����
		 * @param Request HTTP���N�G�X�g
		 */
		void AppendBasicAuthHeader(TSharedRef<IHttpRequest> Request) const;

	private:
		const FString UserPasswordForBasicAuth;
		const FString ServiceURI;
		const FString AddURI;
		const FString GetListURI;
	};

	/**
	 * @brief ���N�G�X�g����
	 */
	class TESTHOSTINGSERVICE_API FRequestResult final
	{
	public:
		FRequestResult(bool bIsSuccess, const FString& Message)
			: bIsSuccess(bIsSuccess), Message(Message)
		{
		}

		/**
		 * @brief ����������
		 */
		bool IsSuccess() const { return bIsSuccess; }

		/**
		 * @brief ���N�G�X�g�ڍ׃��b�Z�[�W
		 *		�@ ���N�G�X�g�Ɏ��s�����ꍇ�̃G���[�̏ڍד�������܂��B
		 */
		const FString& GetMessage() const { return Message; }
	private:
		const bool bIsSuccess;
		const FString Message;
	};

	/**
	 * @brief �e�X�g�P�[�X�f�[�^
	 */
	class TESTHOSTINGSERVICE_API FTestCaseData final
	{
	public:
		FTestCaseData() = default;

		/**
		 * @brief �R���X�g���N�^
		 * @param Name �e�X�g�P�[�X��
		 * @param Commands �e�X�g�R�}���h�z��
		 * @param Summary �e�X�g�P�[�X�T�}�����
		 */
		FTestCaseData(const FString& Name, const TArray<FString>& Commands, const FString& Summary);

		const FString& GetName() const { return Name; }
		const TArray<FString>& GetCommands() const { return Commands; }
		const FString& GetSummary() const { return Summary; }

	private:
		const FString Name;
		const TArray<FString> Commands;
		const FString Summary;
	};

	/**
	 * @brief �e�X�g�P�[�X�f�[�^�擾���N�G�X�g
	 */
	class TESTHOSTINGSERVICE_API FGetTestCaseDataRequest final
	{
	public:

		/**
		 * @brief �e�X�g�P�[�X�f�[�^�擾���N�G�X�g���s���@���������s����܂��B
		 * @param TestCaseName �擾�������e�X�g�P�[�X�f�[�^�̖��O
		 * @param Context �F�؏��Ȃǂ̃R���e�L�X�g
		 * @return ���N�G�X�g����
		 */
		FRequestResult Request(const FString& TestCaseName, const FContext& Context);

		/**
		 * @brief ���N�G�X�g���s�ς݂ŗL���ȃf�[�^�������Ă��邩
		 *         ��x�����N�G�X�g���s���Ă��Ȃ��ꍇ�L���f�[�^���Ȃ�����false��Ԃ��܂��B
		 */
		bool IsValid() const;
		
		/**
		 * @brief ���N�G�X�g���ʂ̎擾
		 *		   IsValid()��false��Ԃ���ԂŌĂяo����ensure���������܂��B
		 */
		const FRequestResult& GetResult() const;

		/**
		 * @brief ���N�G�X�g�ɂ���Ď擾�����e�X�g�P�[�X�f�[�^�̎擾
		 *         IsValid()��false��Ԃ���ԂŌĂяo����ensure���������܂��B
		 */
		const FTestCaseData& GetTestCaseData() const;
		
	private:
		void OnGetRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

		TOptional<FRequestResult> Result;
		TOptional<FTestCaseData> TestCaseData;
	};

	
	/**
	 * @brief �e�X�g�P�[�X�f�[�^�ǉ��E�X�V���N�G�X�g
	 *		�@�����̃e�X�g�P�[�X�f�[�^�ɑ΂��đ���ƍX�V�����ƂȂ�܂�
	 */
	class TESTHOSTINGSERVICE_API FAddTestCaseDataRequest final
	{
	public:

		/**
		 * @brief �e�X�g�P�[�X�f�[�^�ǉ��E�X�V���N�G�X�g���s��
		 * @param TestCaseData �Ώۃe�X�g�P�[�X�f�[�^
		 * @param Context �F�؏��Ȃǂ̃R���e�L�X�g
		 * @return ���N�G�X�g����
		 */
		FRequestResult Request(const FTestCaseData& TestCaseData, const FContext& Context);

		/**
		 * @brief ���N�G�X�g���s�ς݂ŗL���ȃf�[�^�������Ă��邩
		 *         ��x�����N�G�X�g���s���Ă��Ȃ��ꍇ�L���f�[�^���Ȃ�����false��Ԃ��܂��B
		 */
		bool IsValid() const;

		/**
		 * @brief ���N�G�X�g���ʂ̎擾
		 *		   IsValid()��false��Ԃ���ԂŌĂяo����ensure���������܂��B
		 */
		const FRequestResult& GetResult() const;

	private:
		void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
		TOptional<FRequestResult> Result;
	};


	/**
	 * @brief �e�X�g�P�[�X�f�[�^���ꗗ�擾���N�G�X�g
	 */
	class TESTHOSTINGSERVICE_API FGetTestCaseListRequest final
	{
	public:

		/**
		 * @brief �e�X�g�P�[�X�f�[�^�ǉ��E�X�V���N�G�X�g���s��
		 * @param Context �F�؏��Ȃǂ̃R���e�L�X�g
		 * @return ���N�G�X�g����
		 */
		FRequestResult Request(const FContext& Context);

		/**
		 * @brief ���N�G�X�g���s�ς݂ŗL���ȃf�[�^�������Ă��邩
		 *         ��x�����N�G�X�g���s���Ă��Ȃ��ꍇ�L���f�[�^���Ȃ�����false��Ԃ��܂��B
		 */
		bool IsValid() const;

		/**
		 * @brief ���N�G�X�g���ʂ̎擾
		 *		   IsValid()��false��Ԃ���ԂŌĂяo����ensure���������܂��B
		 */
		const FRequestResult& GetResult() const;
		
		/**
		 * @brief �e�X�g�P�[�X�f�[�^���ꗗ�擾
		 *		   IsValid()��false��Ԃ���ԂŌĂяo����ensure���������܂��B
		 */
		const TArray<FString>& GetTestCaseNames() const;

	private:
		void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
		TOptional<FRequestResult> Result;
		TOptional<TArray<FString>> TestCaseNames;

		static const TArray<FString> InvalidTestCaseNames;
	};
	
};

