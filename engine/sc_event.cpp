// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"

// ==========================================================================
// Event Memory Management
// ==========================================================================

void* event_t::allocate( std::size_t size, sim_t* sim )
{
  static const std::size_t SIZE = 2 * sizeof( event_t );
  assert( SIZE > size ); ( void ) size;

  event_t* e = sim -> recycled_event_list;

  if( e )
  {
    sim -> recycled_event_list = e -> next;
  }
  else
  {
    e = (event_t*) ::operator new( SIZE );
  }

  return e;
}

void event_t::recycle( event_t* e )
{
  e -> next = e -> sim.recycled_event_list;
  e -> sim.recycled_event_list = e;
}

// ==========================================================================
// Event
// ==========================================================================

event_t::event_t( sim_t& s, const char* n ) :
  sim( s ), player( 0 ), name( n ), time( timespan_t::zero() ),
  reschedule_time( timespan_t::zero() ), id( 0 ), canceled( false ), next( 0 )
{}

event_t::event_t( sim_t& s, player_t* p, const char* n ) :
  sim( s ), player( p ), name( n ), time( timespan_t::zero() ),
  reschedule_time( timespan_t::zero() ), id( 0 ), canceled( false ), next( 0 )
{}

event_t::event_t( player_t* p, const char* n ) :
  sim( *( p -> sim ) ), player( p ), name( n ), time( timespan_t::zero() ),
  reschedule_time( timespan_t::zero() ), id( 0 ), canceled( false ), next( 0 )
{}

// event_t::reschedule ======================================================

void event_t::reschedule( timespan_t new_time )
{
  reschedule_time = sim.current_time + new_time;

  if ( sim.debug )
    sim.output( "Rescheduling event %s (%d) from %.2f to %.2f",
                name, id, time.total_seconds(), reschedule_time.total_seconds() );
}

// event_t::cancel ==========================================================

void event_t::cancel( event_t*& e )
{
  if( ! e ) return;
  if ( e -> player && ! e -> canceled )
  {
    e -> player -> events--;
#ifndef NDEBUG
    if ( e -> player -> events < 0 )
    {
      e -> sim.errorf( "event_t::cancel assertion error: e -> player -> events < 0, event %s from %s.\n",
                       e -> name, e -> player -> name() );
      assert( 0 );
    }
#endif
  }
  e -> canceled = true;
  e = 0;
}

// event_t::early ===========================================================

void event_t::early( event_t*& e )
{
  if( ! e ) return;
  if ( e -> player && ! e -> canceled )
  {
    e -> player -> events--;
    assert( e -> player -> events >= 0 );
  }
  e -> canceled = true;
  e -> execute();
  e = 0;
}
