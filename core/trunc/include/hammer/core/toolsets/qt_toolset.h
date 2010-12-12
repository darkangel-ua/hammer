#ifndef QT_TOOLSET_H_
#define QT_TOOLSET_H_

#include <hammer/core/toolset.h>

namespace hammer{

class qt_toolset : public toolset
{
	public:
    	qt_toolset();
    	virtual void autoconfigure(engine& e) const;

	protected:
    	virtual void init_impl(engine& e, const std::string& version_id = std::string(),
    						   const location_t* toolset_home = NULL) const;
};

class type_tag;

extern const type_tag qt_mockable;
extern const type_tag qt_ui;
extern const type_tag qt_rc;

}

#endif /* QT_TOOLSET_H_ */
