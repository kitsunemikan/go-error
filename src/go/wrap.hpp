#pragma once

#include <go/error.hpp>

namespace go
{
	inline bool is_error(error err, error target)
	{
		if (!err && !target)
			return true;

		if (!err && target)
			return false;

		if (err && !target)
			return false;

		static std::vector<error> errWalk(4); // 4 is arbitrary

		errWalk.resize(1);
		errWalk[0] = err;

		int errId = 0;
		while (errId < errWalk.size())
		{
			auto& errRef = errWalk[errId];

			if (!errRef)
			{
				errId++;
				continue;
			}

			if (errRef == target)
				return true;

			if (errRef.is(target))
				return true;

			auto unwrapped = errRef.unwrap();
			if (unwrapped)
			{
				errRef = unwrapped;
				continue;
			}

			auto& unwrappedErrs = errRef.unwrap_multiple();
			if (unwrappedErrs.size() == 0)
			{
				errId++;
				continue;
			}

			errRef = unwrappedErrs[0];

			for (size_t i = 1; i < unwrappedErrs.size(); i++)
				errWalk.push_back(unwrappedErrs[i]);
		}

		return false;
	}
}