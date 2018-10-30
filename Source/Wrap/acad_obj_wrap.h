#ifndef _ACAD_OBJ_WRAP_H_
#define _ACAD_OBJ_WRAP_H_

	template <typename T>
	struct ObjectWrap
	{
		T* object;

		ObjectWrap(T*);
		~ObjectWrap();

		ObjectWrap(const ObjectWrap&);
		ObjectWrap();

	private:
		const ObjectWrap& operator=(const ObjectWrap&);
	};


	template<typename T>
	ObjectWrap<T>::ObjectWrap(T * t) :
		object(t)
	{
	}

	template<typename T>
	ObjectWrap<T>::~ObjectWrap()
	{
		if (NULL != object)
			object->close();
	}

	template<typename T>
	 ObjectWrap<T>::ObjectWrap(const ObjectWrap & obj_wrap) :
		object(obj_wrap.object)
	{
	}

	template<typename T>
	 ObjectWrap<T>::ObjectWrap() :
		object(NULL)
	{
	}

	/*template<typename T>
	 const ObjectWrap & ObjectWrap<T>::operator=(const ObjectWrap & obj_wrap)
	{
		object = obj_wrap.object;
		return this;
	}*/


#endif
