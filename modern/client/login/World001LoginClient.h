#pragma once

// WORLD-001: the client-facing login operation.
//
// This is the "Login(username, password)" surface the brief asks for, kept
// deliberately thin. It builds a LoginRequest and hands it to the protocol
// encoder; it does not know that TEA exists, and it holds no socket.
//
// The transport call is the caller's. The legacy client sends one raw
// NET_MSG_GENERIC per ::send (CNetClient::SendBuffer2, s_NetClient.cpp:815-831),
// so a modern client does the same: EncodeLoginRequest returns bytes, the caller
// writes them, and nothing batches or compresses them.
//
// Two values a UI would otherwise have to invent are supplied here, because both
// are protocol facts rather than presentation:
//
//   randomPassword  the server pushed NET_RANDOMPASS_NUMBER on accept and checks
//                   the echoed value (s_CAgentServerMsgLogin.cpp:745)
//   encryptKey      the pre-shared passphrase the Login server published over the
//                   backbone and the Agent relayed (V028 B.4)
//
// This type deliberately does NOT select a garbage token. Which of the five the
// client sends is a legacy implementation detail of GetGarbageMsg
// (s_NetClient.cpp:872-891), which alternates between two slots and avoids
// repeating the previous one. World001LoginClient exposes that choice through
// DeterministicToken so a test is reproducible; a real UI may pass any of the
// five.

#include "LoginProtocol.h"
#include "MinTeaCodec.h"

#include "types/Result.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Client
{
	// What the caller supplies to log in.
	struct LoginRequestData
	{
		std::string userId;
		std::string password;
		std::string randomPassword; // echoed from the server's random number
		std::string encryptKey;     // shared passphrase issued by this cluster
		std::int32_t channel = 0;
	};

	// Builds the bytes for one login attempt.
	//
	// Deterministic: the garbage token is supplied, never generated from a clock
	// or RNG, so the same inputs always produce the same bytes and a test can
	// assert on them.
	class World001LoginClient
	{
	public:
		World001LoginClient() : m_tea() {}
		explicit World001LoginClient(const Network::MinTea& tea) : m_tea(tea) {}

		// Encodes the login request. Returns bytes ready for a single raw write.
		Status EncodeLoginRequest(const LoginRequestData& data,
		                                   const std::string& garbageToken,
		                                   std::vector<Network::WireU8>& out) const
		{
			Network::LoginRequest request;
			request.userId = data.userId;
			request.password = data.password;
			request.randomPassword = data.randomPassword;
			request.encryptKey = data.encryptKey;
			request.channel = data.channel;
			return Network::LoginProtocol::Encode(request, garbageToken, m_tea, out);
		}

		// A token that is always valid, chosen without any RNG.
		//
		// Index is reduced modulo the table so any caller gets a legal token and
		// no caller has to know the five strings.
		static std::string DeterministicToken(std::size_t index)
		{
			const auto& tokens = Network::LoginProtocol::GarbageTokens();
			return tokens[index % tokens.size()];
		}

		const Network::MinTea& Cipher() const noexcept { return m_tea; }

	private:
		Network::MinTea m_tea;
	};
}