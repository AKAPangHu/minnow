#include "reassembler.hh"

using namespace std;

/**
 * 实现的基本思路：
 * 1. 检查是否有立即结束流的数据，如果有则直接结束流。
 * 2. 检查是否超出容量，如果有则截断。
 * 3. 检查是否有重叠，如果有则截断
 * 4. 检查是否可以立即发送，如果有则立即发送。（同时清理内存 + 检查内存后边的数据）
 * 5. 不能立即发送则代表需要缓存，将数据插入到内部存储中。
 */
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  // 恰好是准备结束的最后一个字符串（防止超出无符号整形范围）
  if ( is_last_substring && data.empty() && first_index == next_index_ ) {
    push_to_writer_stream( data, is_last_substring );
    return;
  }

  uint64_t end_index;

  if ( data.empty() ) {
    end_index = first_index;
  } else {
    end_index = first_index + data.size() - 1;
  }

  // 抛弃已经接收到的部分
  if ( end_index < next_index_ ) {
    return;
  }

  // 抛弃部分与需要接受部分重叠
  if ( first_index < next_index_ && end_index >= next_index_ ) {
    data = data.substr( next_index_ - first_index );
    first_index = next_index_;
  }

  // 超出容量处理
  if ( beyond_capacity( end_index ) ) {
    data = data.substr( 0, next_index_ + output_.writer().available_capacity() - first_index );
  }

  if ( is_last_substring ) {
    if ( data.empty() ) {
      last_index = first_index - 1;
    } else {
      last_index = end_index;
    }
  }

  // 如果是符合条件的应该立即发送
  if ( first_index == next_index_ ) {
    push_to_writer_stream( data, is_last_substring );
    next_index_ += data.size();
    check_and_write_from_internal();
  } else {
    insert_into_internal( first_index, data );
  }
}

void Reassembler::insert_into_internal( uint64_t first_index, const string& data )
{
  if ( data.empty() ) {
    return;
  }

  uint64_t end_index = first_index + data.size() - 1;

  for ( auto it = internal_storage_new.begin(); it != internal_storage_new.end(); ) {

    uint64_t item_end_index = it->first + it->second.size() - 1;

    // 处于新字符串左方，继续循环
    if ( first_index < it->first && end_index < it->first ) {
      ++it;
    }

    // 新旧字符串在左侧重叠
    else if ( first_index <= it->first && end_index <= item_end_index ) {
      auto p = merge( first_index, data, it->first, it->second );
      internal_storage_new.insert( it, p );
      internal_storage_new.erase( it );
      return;
    }

    // 新字符串包裹住旧字符串，删除旧字符串
    else if ( first_index < it->first && end_index > item_end_index ) {
      auto p = merge( first_index, data, it->first, it->second );
      it = internal_storage_new.erase( it );
    }

    // 新字符串被包含住
    else if ( first_index >= it->first && end_index <= item_end_index ) {
      return;
    }

    // 新字符串右侧重叠
    else if ( first_index >= it->first && end_index >= item_end_index ) {
      auto p = merge( first_index, data, it->first, it->second );
      internal_storage_new.insert( it, p );
      it = internal_storage_new.erase( it );
      ++it;
    }

    // 新字符串在右侧
    else if ( first_index > item_end_index ) {
      ++it;
    }
  }

  // 合并到最后，没插入的话则直接插入
  internal_storage_new.insert( pair( first_index, data ) );
}

std::pair<uint64_t, std::string> Reassembler::merge( uint64_t old_first_index,
                                                     const std::string& old_string,
                                                     uint64_t new_first_index,
                                                     const std::string& new_string )
{
  uint64_t old_end_index = old_first_index + old_string.size() - 1;
  uint64_t new_end_index = new_first_index + new_string.size() - 1;

  uint64_t front_first_index = min( old_first_index, new_first_index );

  if ( old_first_index <= new_first_index && old_end_index >= new_first_index ) {
    return { old_first_index, old_string };
  } else if ( new_first_index <= old_first_index && new_end_index >= old_first_index ) {
    return { new_first_index, new_string };
  } else {
    string front = old_first_index <= new_first_index ? old_string : new_string;
    string back = old_first_index <= new_first_index ? new_string : old_string;
    uint64_t front_end_index = old_first_index <= new_first_index ? old_end_index : new_end_index;
    uint64_t back_first_index = old_first_index <= new_first_index ? new_first_index : old_first_index;
    return { front_first_index, front + back.substr( front_end_index - back_first_index + 1 ) };
  }
}

void Reassembler::check_and_write_from_internal()
{
  string data;

  for ( auto it = internal_storage_new.begin(); it != internal_storage_new.end();) {
    if ( it->first == next_index_ ) {
      data.append( it->second );
      next_index_ += it->second.size();
      internal_storage_new.erase( it );
      break;
    } else if ( it->first < next_index_ ) {
      uint64_t it_end_index = it->first + it->second.size() - 1;
      if ( it_end_index >= next_index_ ) {
        const basic_string substr = it->second.substr( next_index_ - it->first );
        data.append( substr );
        next_index_ += substr.size();
        internal_storage_new.erase( it );
        break;
      } else {
        it = internal_storage_new.erase( it );
        ++it;
      }
    } else {
      break;
    }
  }

  bool is_last_substring = ( next_index_ > last_index );
  push_to_writer_stream( data, is_last_substring );
}

bool Reassembler::beyond_capacity( uint64_t end_index )
{
  if ( end_index >= next_index_ + output_.writer().available_capacity() ) {
    return true;
  }
  return false;
}

void Reassembler::push_to_writer_stream( const string& data, bool is_last_substring )
{
  if ( !data.empty() ) {
    output_.writer().push( data );
  }

  if ( is_last_substring ) {
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return internal_storage_new.size();
}
