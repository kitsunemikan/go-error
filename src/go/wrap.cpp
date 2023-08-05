#include <go/wrap.hpp>

namespace go
{
	bool is_error(error err, error target)
	{
		if (!err && !target)
			return true;

		if (!err && target)
			return false;

		if (err && !target)
			return false;

		struct dfsStep
		{
			error err;
			int nextChildId;
		};
		static std::vector<dfsStep> errWalk(4); // 4 is arbitrary

		errWalk.resize(1);
		errWalk[0] = {err, 0};

		while (!errWalk.empty())
		{
			auto& errRef = errWalk.back();

			if (!errRef.err)
			{
				errWalk.pop_back();
				continue;
			}

			if (errRef.err == target)
				return true;

			if (errRef.err.is(target))
				return true;

			go::error unwrapped;

			// Skip checking unwrap if we already know that
			// unwrap_multiple is implemented
			if (errRef.nextChildId == 0)
				unwrapped = errRef.err.unwrap();

			if (unwrapped)
			{
				errRef = {unwrapped, 0};
				continue;
			}

			auto& unwrappedErrs = errRef.err.unwrap_multiple();
			if (unwrappedErrs.size() == 0)
			{
				errWalk.pop_back();
				continue;
			}

			if (errRef.nextChildId == unwrappedErrs.size())
			{
				errWalk.pop_back();
				continue;
			}

			errWalk.push_back({unwrappedErrs[errRef.nextChildId], 0});
			errRef.nextChildId++;
		}

		return false;
	}
}