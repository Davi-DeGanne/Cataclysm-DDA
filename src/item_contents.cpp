#include "item_contents.h"

#include "generic_factory.h"
#include "item.h"
#include "item_pocket.h"
#include "optional.h"
#include "units.h"

void item_contents::load( const JsonObject &jo )
{
    optional( jo, was_loaded, "nestable", nestable, true );
    mandatory( jo, was_loaded, "contents", contents );
}

void item_contents::serialize( JsonOut &json ) const
{
    json.start_object();

    json.member( "nestable", nestable );
    json.member( "contents", contents );

    json.end_object();
}

void item_contents::deserialize( JsonIn &jsin )
{
    JsonObject data = jsin.get_object();
    load( data );
}

bool item_contents::stacks_with( const item_contents &rhs ) const
{
    if( contents.size() != rhs.contents.size() ) {
        return false;
    }
    return std::equal( contents.begin(), contents.end(),
                       rhs.contents.begin(),
    []( const item_pocket & a, const item_pocket & b ) {
        return a.stacks_with( b );
    } );
}

bool item_contents::can_contain( const item &it ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.can_contain( it ) ) {
            return true;
        }
    }
    return false;
}

bool item_contents::empty() const
{
    if( contents.empty() ) {
        return true;
    }
    for( const item_pocket &pocket : contents ) {
        if( pocket.empty() ) {
            return true;
        }
    }
    return false;
}

std::list<item *> item_contents::all_items()
{
    std::list<item *> item_list;
    for( item_pocket &pocket : contents ) {
        std::list<item *> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

std::list<const item *> item_contents::all_items() const
{
    std::list<const item *> item_list;
    for( const item_pocket &pocket : contents ) {
        std::list<item *> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

units::volume item_contents::item_size_modifier() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        total_vol += pocket.item_size_modifier();
    }
    return total_vol;
}

units::mass item_contents::item_weight_modifier() const
{
    units::mass total_mass = 0_gram;
    for( const item_pocket &pocket : contents ) {
        total_mass += pocket.item_weight_modifier();
    }
    return total_mass;
}

cata::optional<item> item_contents::remove_item( const item &it )
{
    for( item_pocket pocket : contents ) {
        cata::optional<item> ret = pocket.remove_item( it );
        if( ret ) {
            return ret;
        }
    }
    return cata::nullopt;
}

cata::optional<item> item_contents::remove_item( const item_location &it )
{
    if( !it ) {
        return cata::nullopt;
    }
    return remove_item( *it );
}

void item_contents::clear_items()
{
    for( item_pocket &pocket : contents ) {
        pocket.clear_items();
    }
}

bool item_contents::insert_item( const item &it )
{
    for( item_pocket pocket : contents ) {
        if( pocket.insert_item( it ) ) {
            return true;
        }
    }
    return false;
}

void item_contents::insert_legacy( const item &it )
{
    for( item_pocket pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            pocket.add( it );
            return;
        }
    }
    item_pocket fake_pocket( item_pocket::pocket_type::LEGACY_CONTAINER );
    fake_pocket.add( it );
    contents.emplace_front( fake_pocket );
}
