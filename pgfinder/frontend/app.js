// app.js - the website. It only shows pages and calls the C++ server (/api/...).
// All searching, filtering, scoring, sorting and booking happens in the C++ backend.
'use strict';

const app = document.getElementById('app');

const state = {
  colleges: [],
  search: null,        // last search preferences entered by the student
  booking: null,       // last booking confirmation
  owner: null,         // logged-in owner {id, name}
  ownerHostels: [],
  showAdd: false,
  openHostel: null,
  collegeError: ''
};
try { state.owner = JSON.parse(localStorage.getItem('owner')); } catch (e) { state.owner = null; }

// details page state
let cur = { hostel: null, sharing: 0, sel: null };

// ---------- helpers ----------
const esc = s => String(s).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const rupee = n => '\u20B9' + Number(n).toLocaleString('en-IN');
const collegeName = id => (state.colleges.find(c => c.id === id) || { name: id }).name;

let toastTimer = null;
function toast(message, ok) {
  const t = document.getElementById('toast');
  t.textContent = message;
  t.className = 'show ' + (ok ? 'ok' : 'err');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { t.className = ''; }, 3500);
}

async function api(path, params, method) {
  method = method || 'GET';
  const qs = new URLSearchParams(params || {}).toString();
  try {
    let res;
    if (method === 'GET') {
      res = await fetch('https://pg-finder-ej1d.onrender.com/api/' + path + (qs ? '?' + qs : ''));
    } else {
      res = await fetch('https://pg-finder-ej1d.onrender.com/api/' + path, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: qs
      });
    }
    return await res.json();
  } catch (e) {
    return { ok: false, error: 'Could not reach the C++ server. Is it running?' };
  }
}

function collegeOptions(selected) {
  return '<option value="">Select college</option>' + state.colleges.map(c =>
    `<option value="${esc(c.id)}" ${c.id === selected ? 'selected' : ''}>${esc(c.name)}</option>`).join('');
}

const tagHtml = (sharing, ac) =>
  `<span class="tag">${sharing} Sharing</span><span class="tag ${ac ? 'ac' : 'nonac'}">${ac ? 'AC' : 'Non-AC'}</span>`;

// ---------- router ----------
window.addEventListener('hashchange', route);

async function route() {
  if (!state.colleges.length) {
    const r = await api('colleges');
    state.colleges = r.colleges || [];
    state.collegeError = state.colleges.length ? '' : 'Could not load colleges. Start the C++ server and open this site at http://localhost:8080.';
  }
  const parts = (location.hash || '#/').slice(2).split('/');
  const activeRoute = parts[0] === 'owner' ? 'owner' :
    ['search', 'results', 'hostel', 'booked'].includes(parts[0]) ? 'search' : 'home';
  document.querySelectorAll('.topbar nav a').forEach(link => {
    const targetRoute = (link.getAttribute('href') || '').slice(2).split('/')[0] || 'home';
    if (targetRoute === activeRoute) link.setAttribute('aria-current', 'page');
    else link.removeAttribute('aria-current');
  });
  window.scrollTo(0, 0);
  switch (parts[0]) {
    case 'search': return viewSearch();
    case 'results': return viewResults();
    case 'hostel': return viewHostel(parts[1]);
    case 'booked': return viewBooked();
    case 'owner': return viewOwner();
    default: return viewHome();
  }
}

// ---------- Home ----------
function viewHome() {
  app.innerHTML = `
    <section class="hero" aria-labelledby="home-title">
      <img class="hero-image" src="https://images.unsplash.com/photo-1555854877-bab0e564b8d5?auto=format&amp;fit=crop&amp;w=1800&amp;q=85" alt="" fetchpriority="high">
      <div class="hero-content">
        <p class="eyebrow">STUDENT LIVING IN PUNE</p>
        <h1 id="home-title">Find the right PG / Hostel near your college</h1>
        <p>Tell us your budget and preferences. We will show suitable hostels in Pune and let you book a bed.</p>
        <div class="actions">
          <a class="btn" href="#/search">Find a Hostel <span aria-hidden="true">&#8594;</span></a>
          <a class="btn secondary" href="#/owner">I'm a Hostel Owner</a>
        </div>
      </div>
    </section>
    <section class="steps" aria-label="How it works">
      <div class="panel"><h3>1. Enter preferences</h3><p class="muted">College, gender, budget, distance, food, sharing and AC.</p></div>
      <div class="panel"><h3>2. See matches</h3><p class="muted">Hostels that fit your preferences.</p></div>
      <div class="panel"><h3>3. Book a bed</h3><p class="muted">Pick a room and an available bed. Get an instant confirmation.</p></div>
    </section>`;
}

// ---------- Student search form ----------
function viewSearch() {
  const s = state.search || { name: '', college: '', gender: 'Male', budget: 10000, maxDist: 3, food: 'Any', sharing: '0', ac: 'Any' };
  const opt = (value, label, selected) => `<option value="${value}" ${String(selected) === String(value) ? 'selected' : ''}>${label}</option>`;
  app.innerHTML = `
    <div class="narrow panel">
      <h2>Find a Hostel</h2>
      <p class="muted">${state.collegeError ? esc(state.collegeError) : 'Fields marked * are required.'}</p>
      ${state.collegeError ? '<button class="btn secondary small" type="button" data-act="retry-colleges">Retry college list</button>' : ''}
      <form data-form="search">
        <div class="form-grid">
          <div><label>Name *</label><input name="name" value="${esc(s.name)}" required></div>
          <div><label>College *</label><select name="college" required ${state.colleges.length ? '' : 'disabled'}>${collegeOptions(s.college)}</select></div>
          <div><label>Gender *</label>
            <div class="radio-row">
              <label><input type="radio" name="gender" value="Male" ${s.gender === 'Male' ? 'checked' : ''}> Male</label>
              <label><input type="radio" name="gender" value="Female" ${s.gender === 'Female' ? 'checked' : ''}> Female</label>
            </div></div>
          <div><label>Maximum monthly budget (\u20B9) *</label><input type="number" name="budget" min="1000" step="500" value="${esc(s.budget)}" required></div>
          <div><label>Maximum distance (km) *</label><input type="number" name="maxDist" min="0.5" step="0.5" value="${esc(s.maxDist)}" required></div>
          <div><label>Food preference *</label><select name="food">
            ${opt('Any', 'No Preference', s.food)}${opt('Veg', 'Veg', s.food)}${opt('Non-Veg', 'Non-Veg', s.food)}</select></div>
          <div><label>Room sharing *</label><select name="sharing">
            ${opt('0', 'No Preference', s.sharing)}${opt('1', '1 Sharing', s.sharing)}${opt('2', '2 Sharing', s.sharing)}${opt('3', '3 Sharing', s.sharing)}</select></div>
          <div><label>AC preference (optional)</label><select name="ac">
            ${opt('Any', 'No Preference', s.ac)}${opt('AC', 'AC', s.ac)}${opt('Non-AC', 'Non-AC', s.ac)}</select></div>
        </div>
        <div class="form-actions"><button class="btn" type="submit" ${state.colleges.length ? '' : 'disabled'}>Search</button></div>
      </form>
    </div>`;
}

// ---------- Results ----------
async function viewResults() {
  if (!state.search) { location.hash = '#/search'; return; }
  app.innerHTML = '<p class="muted">Searching...</p>';
  const r = await api('search', state.search);
  if (!r.ok) { toast(r.error); location.hash = '#/search'; return; }

  const head = `
    <div class="result-head">
      <div>
        <h2>Hostels near ${esc(r.college)}</h2>
        <p class="muted">${r.count} suitable ${state.search.gender.toLowerCase()} hostel${r.count === 1 ? '' : 's'}</p>
      </div>
      <a class="btn secondary small" href="#/search">Edit search</a>
    </div>`;

  if (r.count === 0) {
    app.innerHTML = head + `<div class="panel empty"><h3>No hostel matches your preferences</h3>
      <p class="muted">Hostels appear when they have an available room that fits your search. Try a higher budget, a larger distance, or "No Preference" for sharing / AC / food.</p></div>`;
    return;
  }

  app.innerHTML = head + '<div class="cards">' + r.results.map((h, index) => `
    <article class="card">
      <div class="card-top">
        <div><h3>${esc(h.name)}</h3><p class="muted small">${esc(h.location)}</p></div>
        <div class="priority ${index < 3 ? `top-${index + 1}` : ''}">${['🥇', '🥈', '🥉'][index] || ''}<span>Priority #${index + 1}</span></div>
      </div>
      <div class="tags"><span class="tag">${esc(h.gender)}</span><span class="tag">${esc(h.food)}</span>${tagHtml(h.room.sharing, h.room.ac)}</div>
      <div class="stats">
        <div><b>${h.distance} km</b><span>from college</span></div>
        <div><b>${rupee(h.room.rent)}</b><span>per month</span></div>
        <div><b>${h.matchingBeds}</b><span>beds available</span></div>
      </div>
      <p class="muted small">Shown: cheapest matching room. ${h.matchingRooms} matching room${h.matchingRooms === 1 ? '' : 's'} in this hostel.</p>
      <a class="btn" href="#/hostel/${h.hostelId}">View &amp; Book</a>
    </article>`).join('') + '</div>';
}

// ---------- Hostel details + booking ----------
async function viewHostel(id) {
  app.innerHTML = '<p class="muted">Loading...</p>';
  const r = await api('hostel', { id: id, college: state.search ? state.search.college : '' });
  if (!r.ok) { toast(r.error); location.hash = state.search ? '#/results' : '#/search'; return; }
  cur = { hostel: r.hostel, sharing: state.search ? Number(state.search.sharing) : 0, sel: null };
  drawHostel();
}

function drawHostel() {
  const h = cur.hostel;
  const s = state.search;
  const rooms = h.rooms.filter(r => cur.sharing === 0 || r.sharing === cur.sharing);
  const tab = (n, label) => `<button class="tab ${cur.sharing === n ? 'active' : ''}" data-act="tab" data-n="${n}">${label}</button>`;

  let panel = '';
  if (cur.sel) {
    const room = h.rooms.find(r => r.id === cur.sel.roomId);
    panel = `
      <div class="panel book-panel">
        <h3>Book Room ${room.id}, Bed ${cur.sel.bedId}</h3>
        <p class="muted">${room.sharing} Sharing \u00B7 ${room.ac ? 'AC' : 'Non-AC'} \u00B7 ${rupee(room.rent)} per month</p>
        <form data-form="book">
          <div class="form-grid">
            <div><label>Your name *</label><input name="name" value="${s ? esc(s.name) : ''}" required></div>
            <div><label>Gender *</label><select name="gender">
              <option value="Male" ${s && s.gender === 'Male' ? 'selected' : ''}>Male</option>
              <option value="Female" ${s && s.gender === 'Female' ? 'selected' : ''}>Female</option></select></div>
            <div><label>College *</label><select name="college" required>${collegeOptions(s ? s.college : '')}</select></div>
            <div><label>Phone number or email *</label><input name="contact" type="text" autocomplete="off" placeholder="e.g. +91 98765 43210 or name@example.com" required></div>
          </div>
          <div class="form-actions"><button class="btn" type="submit">Confirm Booking</button></div>
        </form>
      </div>`;
  }

  app.innerHTML = `
    <p><a href="${s ? '#/results' : '#/search'}">&larr; Back</a></p>
    <div class="panel">
      <h2>${esc(h.name)}</h2>
      <p class="muted">${esc(h.location)}, Pune</p>
      <div class="info-grid">
        <div><span>Gender</span><b>${esc(h.gender)}</b></div>
        <div><span>Food</span><b>${esc(h.food)}</b></div>
        ${s && h.distance !== undefined ? `<div><span>Distance from ${esc(collegeName(s.college))}</span><b>${h.distance} km</b></div>` : ''}
        <div><span>Beds available</span><b>${h.availableBeds}</b></div>
      </div>
    </div>

    <h2 style="margin-top:1.5rem">Rooms &amp; beds</h2>
    <p class="muted small">Choose a sharing type, then tap a green bed to select it.</p>
    <div class="tabs">${tab(0, 'All')}${tab(1, '1 Sharing')}${tab(2, '2 Sharing')}${tab(3, '3 Sharing')}</div>
    <div class="rooms">${rooms.length ? rooms.map(r => `
      <div class="room">
        <div class="room-head"><b>Room ${r.id}</b>${tagHtml(r.sharing, r.ac)}<span class="rent">${rupee(r.rent)}</span></div>
        <div class="beds">${r.beds.map(b => {
          const chosen = cur.sel && cur.sel.roomId === r.id && cur.sel.bedId === b.id;
          return `<button class="bed ${b.occupied ? 'taken' : ''} ${chosen ? 'selected' : ''}" ${b.occupied ? 'disabled' : ''}
                    data-act="pick-bed" data-room="${r.id}" data-bed="${b.id}">Bed ${b.id}</button>`;
        }).join('')}</div>
        <span class="muted small">${r.availableBeds} of ${r.capacity} beds available</span>
      </div>`).join('') : '<p class="muted">No rooms of this type.</p>'}</div>
    ${panel}`;
}

function viewBooked() {
  if (!state.booking) { location.hash = '#/'; return; }
  const b = state.booking;
  app.innerHTML = `
    <div class="panel confirm">
      <div class="tick">\u2713</div>
      <h2>Booking Confirmed</h2>
      <p class="muted">Your bed has been reserved. (This is a demo, so no payment is needed.)</p>
      <div class="details">
        <div><span>Booking ID</span><b>#${b.id}</b></div>
        <div><span>Student</span><b>${esc(b.studentName)}</b></div>
        <div><span>Gender</span><b>${esc(b.gender || 'Not provided')}</b></div>
        <div><span>Contact</span><b>${esc(b.contact)}</b></div>
        <div><span>College</span><b>${esc(collegeName(b.college))}</b></div>
        <div><span>Hostel</span><b>${esc(b.hostelName)}</b></div>
        <div><span>Room / Bed</span><b>Room ${b.roomId}, Bed ${b.bedId}</b></div>
        <div><span>Monthly rent</span><b>${rupee(b.rent)}</b></div>
        <div><span>Booked on</span><b>${esc(b.time)}</b></div>
      </div>
      <div class="form-actions" style="justify-content:center">
        <a class="btn" href="#/results">Back to results</a>
        <a class="btn secondary" href="#/">Home</a>
      </div>
    </div>`;
}

// ---------- Owner ----------
async function viewOwner() {
  if (!state.owner) {
    app.innerHTML = `
      <div class="narrow panel">
        <h2>Hostel Owner Login</h2>
        <p class="muted">Enter your name. New names are registered automatically. To see sample hostels, log in as <b>Rajesh Patil</b>.</p>
        <form data-form="owner-login">
          <label>Your name</label><input name="name" required>
          <div class="form-actions"><button class="btn" type="submit">Continue</button></div>
        </form>
      </div>`;
    return;
  }
  app.innerHTML = '<p class="muted">Loading...</p>';
  await loadOwnerHostels();
  drawOwner();
}

async function loadOwnerHostels() {
  const r = await api('owner/hostels', { ownerId: state.owner.id });
  if (!r.ok) { toast(r.error); state.owner = null; localStorage.removeItem('owner'); return; }
  state.ownerHostels = r.hostels;
}

function hostelFields(h) {
  h = h || { name: '', location: '', gender: 'Male', food: 'Veg', distances: {} };
  return `
    <div class="form-grid">
      <div><label>Hostel name</label><input name="name" value="${esc(h.name)}" required></div>
      <div><label>Location / area</label><input name="location" value="${esc(h.location)}" required></div>
      <div><label>Gender type</label><select name="gender">
        <option ${h.gender === 'Male' ? 'selected' : ''}>Male</option><option ${h.gender === 'Female' ? 'selected' : ''}>Female</option></select></div>
      <div><label>Food type</label><select name="food">
        <option ${h.food === 'Veg' ? 'selected' : ''}>Veg</option><option ${h.food === 'Veg + Non-Veg' ? 'selected' : ''}>Veg + Non-Veg</option></select></div>
    </div>
    <p style="margin:1rem 0 .4rem"><b>Distance (km) from each college</b></p>
    <div class="form-grid">${state.colleges.map(c => `
      <div><label>${esc(c.name)}</label>
      <input type="number" step="0.1" min="0.1" name="d_${esc(c.id)}" value="${h.distances[c.id] !== undefined ? h.distances[c.id] : ''}" required></div>`).join('')}
    </div>`;
}

function drawOwner() {
  if (!state.owner) { viewOwner(); return; }
  const addForm = state.showAdd ? `
    <div class="panel" style="margin-bottom:1rem">
      <h3>Add a new hostel</h3>
      <form data-form="hostel-add">${hostelFields()}
        <div class="form-actions"><button class="btn" type="submit">Add Hostel</button>
        <button class="btn secondary" type="button" data-act="toggle-add">Cancel</button></div>
      </form>
      <p class="muted small">After adding the hostel, click Manage to add its rooms.</p>
    </div>` : '';

  app.innerHTML = `
    <div class="owner-bar">
      <div><h2>Owner Dashboard</h2><p class="muted">Logged in as ${esc(state.owner.name)}</p></div>
      <div><button class="btn small" data-act="toggle-add">+ Add Hostel</button>
      <button class="btn secondary small" data-act="owner-logout">Log out</button></div>
    </div>
    ${addForm}
    ${state.ownerHostels.length === 0 ? '<div class="panel empty"><p class="muted">You have no hostels yet. Click "+ Add Hostel".</p></div>' : ''}
    ${state.ownerHostels.map(h => ownerHostelCard(h)).join('')}`;
}

function ownerHostelCard(h) {
  const open = state.openHostel === h.id;
  const bookings = h.bookings || [];
  const bookingList = bookings.length ? `
    <section class="booking-list">
      <h4>Confirmed bookings (${bookings.length})</h4>
      ${bookings.map(b => `
        <div class="booking-item">
          <div><span>Student</span><b>${esc(b.studentName)}</b></div>
          <div><span>Contact</span><b>${esc(b.contact || 'Not provided')}</b></div>
          <div><span>Gender</span><b>${esc(b.gender || 'Not provided')}</b></div>
          <div><span>College</span><b>${esc(collegeName(b.college))}</b></div>
          <div><span>Room / Bed</span><b>Room ${b.roomId}, Bed ${b.bedId}</b></div>
          <div><span>Monthly rent</span><b>${rupee(b.rent)}</b></div>
          <div><span>Booked on</span><b>${esc(b.time)}</b></div>
        </div>`).join('')}
    </section>` : '<p class="muted small booking-empty">No confirmed bookings yet.</p>';
  return `
    <div class="panel owner-hostel">
      <div class="owner-hostel-head">
        <div><h3>${esc(h.name)}</h3>
          <p class="muted small">${esc(h.location)} \u00B7 ${esc(h.gender)} \u00B7 ${esc(h.food)} \u00B7 ${h.rooms.length} rooms \u00B7 ${h.availableBeds} beds available</p></div>
        <div>
          <button class="btn small" data-act="toggle-manage" data-id="${h.id}">${open ? 'Close' : 'Manage'}</button>
          <button class="btn danger small" data-act="hostel-remove" data-id="${h.id}">Remove</button>
        </div>
      </div>
      ${h.availableBeds === 0 ? '<p class="muted small">Not shown in student search until a room has an available bed.</p>' : ''}
      ${bookingList}
      ${open ? manageHtml(h) : ''}
    </div>`;
}

function manageHtml(h) {
  const rows = h.rooms.map(r => `
    <form class="room-row" data-form="room-update">
      <input type="hidden" name="hostelId" value="${h.id}"><input type="hidden" name="roomId" value="${r.id}">
      <div class="label">Room ${r.id}<br><span class="muted small">${r.sharing} Sharing</span></div>
      <div><label>Type</label><select name="ac"><option value="0" ${r.ac ? '' : 'selected'}>Non-AC</option><option value="1" ${r.ac ? 'selected' : ''}>AC</option></select></div>
      <div><label>Rent (\u20B9)</label><input type="number" name="rent" min="6000" max="20000" step="500" value="${r.rent}"></div>
      <div><label>Beds available (0-${r.capacity})</label><input type="number" name="availableBeds" min="0" max="${r.capacity}" value="${r.availableBeds}"></div>
      <div class="btns"><button class="btn small" type="submit">Update</button>
        <button class="btn danger small" type="button" data-act="room-remove" data-hostel="${h.id}" data-room="${r.id}">Delete</button></div>
    </form>`).join('');

  return `
    <div class="manage">
      <h3>Hostel details</h3>
      <form data-form="hostel-update"><input type="hidden" name="hostelId" value="${h.id}">${hostelFields(h)}
        <div class="form-actions"><button class="btn" type="submit">Save details</button></div></form>

      <h3 style="margin-top:1.5rem">Rooms</h3>
      ${rows || '<p class="muted">No rooms yet. Add some below.</p>'}

      <h3 style="margin-top:1.5rem">Add rooms</h3>
      <form data-form="room-add"><input type="hidden" name="hostelId" value="${h.id}">
        <div class="form-grid">
          <div><label>Sharing type</label><select name="sharing"><option value="1">1 Sharing</option><option value="2" selected>2 Sharing</option><option value="3">3 Sharing</option></select></div>
          <div><label>AC / Non-AC</label><select name="ac"><option value="0">Non-AC</option><option value="1">AC</option></select></div>
          <div><label>Rent per bed (\u20B9, 6000 - 20000)</label><input type="number" name="rent" min="6000" max="20000" step="500" value="8000" required></div>
          <div><label>Number of rooms</label><input type="number" name="count" min="1" max="10" value="1" required></div>
        </div>
        <div class="form-actions"><button class="btn" type="submit">Add rooms</button></div>
      </form>
    </div>`;
}

async function ownerCall(path, params, message) {
  params.set('ownerId', state.owner.id);
  const r = await api(path, params, 'POST');
  if (!r.ok) { toast(r.error); return false; }
  toast(message, true);
  await loadOwnerHostels();
  drawOwner();
  return true;
}

// ---------- events (forms) ----------
document.addEventListener('submit', async e => {
  e.preventDefault();
  const form = e.target;
  const kind = form.dataset.form;
  const data = new FormData(form);
  const params = new URLSearchParams(data);

  if (kind === 'search') {
    state.search = Object.fromEntries(data.entries());
    if (location.hash === '#/results') route();
    else location.hash = '#/results';

  } else if (kind === 'book') {
    params.set('hostelId', cur.hostel.id);
    params.set('roomId', cur.sel.roomId);
    params.set('bedId', cur.sel.bedId);
    const r = await api('book', params, 'POST');
    if (r.ok) {
      state.booking = r.booking;
      location.hash = '#/booked';
    } else {
      toast(r.error);
      viewHostel(cur.hostel.id);   // reload: the bed may have been taken
    }

  } else if (kind === 'owner-login') {
    const r = await api('owner/login', params, 'POST');
    if (!r.ok) return toast(r.error);
    state.owner = r.owner;
    localStorage.setItem('owner', JSON.stringify(r.owner));
    viewOwner();

  } else if (kind === 'hostel-add') {
    params.set('ownerId', state.owner.id);
    const r = await api('owner/hostel/add', params, 'POST');
    if (!r.ok) return toast(r.error);
    state.showAdd = false;
    state.openHostel = Number(r.hostelId);
    toast('Hostel saved. Add a room with available beds to make it appear in student search.', true);
    await loadOwnerHostels();
    drawOwner();

  } else if (kind === 'hostel-update') {
    await ownerCall('owner/hostel/update', params, 'Hostel details saved.');

  } else if (kind === 'room-add') {
    await ownerCall('owner/room/add', params, 'Rooms added.');

  } else if (kind === 'room-update') {
    await ownerCall('owner/room/update', params, 'Room updated.');
  }
});

// ---------- events (buttons) ----------
document.addEventListener('click', async e => {
  const el = e.target.closest('[data-act]');
  if (!el) return;
  const act = el.dataset.act;

  if (act === 'retry-colleges') {
    state.colleges = [];
    await route();

  } else if (act === 'tab') {
    cur.sharing = Number(el.dataset.n);
    cur.sel = null;
    drawHostel();

  } else if (act === 'pick-bed') {
    cur.sel = { roomId: Number(el.dataset.room), bedId: Number(el.dataset.bed) };
    drawHostel();

  } else if (act === 'toggle-add') {
    state.showAdd = !state.showAdd;
    drawOwner();

  } else if (act === 'toggle-manage') {
    const id = Number(el.dataset.id);
    state.openHostel = state.openHostel === id ? null : id;
    drawOwner();

  } else if (act === 'owner-logout') {
    state.owner = null;
    localStorage.removeItem('owner');
    viewOwner();

  } else if (act === 'hostel-remove') {
    if (confirm('Remove this hostel? It will disappear from student recommendations.'))
      await ownerCall('owner/hostel/remove', new URLSearchParams({ hostelId: el.dataset.id }), 'Hostel removed.');

  } else if (act === 'room-remove') {
    if (confirm('Delete this room?'))
      await ownerCall('owner/room/remove', new URLSearchParams({ hostelId: el.dataset.hostel, roomId: el.dataset.room }), 'Room deleted.');
  }
});

route();
