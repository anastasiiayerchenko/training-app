// Configuration
const API_URLS = {
    auth: '/api/auth',
    training: '/api/training',
    records: '/api/records'
};

// State
let currentUser = null;
let isLoginMode = true; // Default to Login Mode

// DOM Elements
const views = {
    landing: document.getElementById('app-container'),
    profile: document.getElementById('profile-view')
};

const btnAuth = document.getElementById('btn-auth');
const toggleLinks = document.querySelectorAll('.toggle-auth-mode');
const registerOnlyFields = document.querySelectorAll('.register-only');
const toastEl = document.getElementById('toast');

// Utility Functions
function showToast(message, isError = false) {
    toastEl.textContent = message;
    toastEl.style.background = isError ? '#ff4d4f' : '#52c41a';
    toastEl.style.display = 'block';
    setTimeout(() => {
        toastEl.style.display = 'none';
    }, 3000);
}

function switchView(viewName) {
    Object.values(views).forEach(v => v.classList.add('hidden'));
    views[viewName].classList.remove('hidden');

    // Always reset the window scroll position to the very top when switching views (e.g. after login or registration)
    window.scrollTo(0, 0);

    // Smoothly apply/remove premium landing page background image
    if (viewName === 'landing') {
        document.body.classList.add('landing-bg');
    } else {
        document.body.classList.remove('landing-bg');
    }
}

// Toggle Auth Mode (Login vs Register)
function toggleAuthMode(e) {
    if (e) e.preventDefault();
    isLoginMode = !isLoginMode;

    if (isLoginMode) {
        btnAuth.textContent = 'Log In';
        toggleLinks.forEach(link => link.textContent = "Don't have an account? Register");
        registerOnlyFields.forEach(field => field.classList.add('hidden'));
    } else {
        btnAuth.textContent = 'Register';
        toggleLinks.forEach(link => link.textContent = 'Already have an account? Log In');
        registerOnlyFields.forEach(field => field.classList.remove('hidden'));
    }
}

toggleLinks.forEach(link => link.addEventListener('click', toggleAuthMode));

// API Call Wrapper
async function apiCall(service, endpoint, method = 'GET', body = null) {
    const url = `${API_URLS[service]}${endpoint}`;
    const options = {
        method,
        headers: { 'Content-Type': 'application/json' }
    };
    if (body) {
        options.body = JSON.stringify(body);
    }

    try {
        const response = await fetch(url, options);
        const data = await response.json();
        if (!response.ok) {
            throw new Error(data.error || 'API Error');
        }
        return data;
    } catch (error) {
        showToast(error.message, true);
        throw error;
    }
}

// Clear error styling when user types in username or password
const loginUsernameEl = document.getElementById('login-username');
const loginPasswordEl = document.getElementById('login-password');
if (loginUsernameEl && loginPasswordEl) {
    const clearErrors = () => {
        loginUsernameEl.classList.remove('input-error');
        loginPasswordEl.classList.remove('input-error');
    };
    loginUsernameEl.addEventListener('input', clearErrors);
    loginPasswordEl.addEventListener('input', clearErrors);
}

// Authentication
btnAuth.addEventListener('click', async () => {
    const usernameInput = document.getElementById('login-username');
    const passwordInput = document.getElementById('login-password');
    const username = usernameInput.value;
    const password = passwordInput.value;

    // Reset error styling on submit click
    usernameInput.classList.remove('input-error');
    passwordInput.classList.remove('input-error');

    if (!username || !password) {
        showToast('Please enter username and password', true);
        return;
    }

    try {
        if (isLoginMode) {
            // LOGIN
            const data = await apiCall('auth', '/login', 'POST', { username, password });
            currentUser = data;
            showToast(`Welcome, ${data.username}!`);
            await loadProfile();
            switchView('profile');
        } else {
            // REGISTER
            const fname = document.getElementById('reg-fname').value;
            const lname = document.getElementById('reg-lname').value;
            const dob = document.getElementById('reg-dob').value;

            if (!fname || !lname || !dob) {
                showToast("Please fill in all required fields for registration", true);
                return;
            }

            const body = {
                first_name: fname,
                last_name: lname,
                dob: dob,
                username: username,
                password: password
            };

            await apiCall('auth', '/register', 'POST', body);
            showToast('Registration successful! Logging in...');
            // Auto login after register
            const loginData = await apiCall('auth', '/login', 'POST', { username, password });
            currentUser = loginData;
            await loadProfile();
            switchView('profile');
        }
    } catch (e) {
        // Error is handled in apiCall
        const errMsg = (e.message || "").toLowerCase();
        if (errMsg.includes("taken") || errMsg.includes("already") || errMsg.includes("exists")) {
            usernameInput.classList.add('input-error');
            usernameInput.focus();
        } else if (errMsg.includes("credentials") || errMsg.includes("invalid") || errMsg.includes("unauthorized")) {
            usernameInput.classList.add('input-error');
            passwordInput.classList.add('input-error');
            usernameInput.focus();
            showToast("Incorrect username or password", true);
        }
    }
});

// Load Profile
async function loadProfile() {
    if (!currentUser) return;
    try {
        const user = await apiCall('auth', `/users/${currentUser.id}`);
        document.getElementById('user-fullname').textContent = `${user.first_name} ${user.last_name}`;

        await loadTrainings();
        await loadRecords();
    } catch (e) { }
}

async function loadTrainings() {
    const listEl = document.getElementById('training-list');
    listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light);">Loading...</div>';

    // Hide details cards initially so they do not show in the background
    document.getElementById('training-modal').classList.add('hidden');
    document.getElementById('record-modal').classList.add('hidden');

    try {
        const data = await apiCall('training', `/athletes/${currentUser.id}/trainings`);
        const trainings = data.trainings || [];

        // Update total training count
        const totalLabel = document.getElementById('total-trainings-label');
        if (totalLabel) totalLabel.textContent = `Total Trainings: ${trainings.length}`;

        if (!trainings || trainings.length === 0) {
            listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light); opacity: 0.6; font-style: italic;">You don\'t have any trainings yet</div>';
            return;
        }

        listEl.innerHTML = '';
        trainings.forEach((tr) => {
            let icon = '🏋️';
            if (tr.type && tr.type.toLowerCase().includes('cardio')) icon = '🏃';
            if (tr.type && tr.type.toLowerCase().includes('full')) icon = '🤸';

            const item = document.createElement('div');
            item.className = 'history-item';

            item.innerHTML = `
                <div class="history-item-thumb">${icon}</div>
                <div class="history-info">
                    <div class="history-category">${tr.type || 'Workout'}</div>
                    <div class="history-date">${tr.date || 'No Date'}</div>
                </div>
                <div class="history-arrow">›</div>
            `;
            item.addEventListener('click', () => {
                // Hide record details when opening workout details
                document.getElementById('record-modal').classList.add('hidden');
                showTrainingModal(tr, item);
            });
            listEl.appendChild(item);
        });
    } catch (e) {
        listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light); opacity: 0.6; font-style: italic;">You don\'t have any trainings yet</div>';
    }
}

function showTrainingModal(tr, activeItem) {
    // Highlight selected row using the active-item class
    document.querySelectorAll('.history-item').forEach(el => el.classList.remove('active-item'));
    if (activeItem) activeItem.classList.add('active-item');

    document.getElementById('modal-type').textContent = 'Workout';
    const muscle = tr.muscle && tr.muscle !== '-' ? ` • ${tr.muscle}` : '';
    document.getElementById('modal-date').textContent = `${tr.type}${muscle} • ${tr.date || ''}`;

    const exEl = document.getElementById('modal-exercises');
    const exercises = tr.exercises || [];

    const icons = { 'Strength': '🏋️', 'Cardio': '🏃', 'Fullbody': '🤸' };
    const icon = icons[tr.type] || '🏋️';

    if (exercises.length === 0) {
        exEl.innerHTML = '<div class="modal-empty">No exercises</div>';
    } else {
        exEl.innerHTML = exercises.map(ex => {
            let details = '';
            if (ex.sets && ex.reps) details = `${ex.sets} sets × ${ex.reps} reps`;
            else if (ex.duration) details = ex.duration;
            return `
                <div class="modal-exercise-item">
                    <div class="modal-exercise-icon">${icon}</div>
                    <div>
                        <div class="modal-exercise-name">${ex.name}</div>
                        ${details ? `<div class="modal-exercise-details">${details}</div>` : ''}
                    </div>
                </div>
            `;
        }).join('');
    }

    document.getElementById('training-modal').classList.remove('hidden');
}

function closeTrainingModal() {
    document.getElementById('training-modal').classList.add('hidden');
    document.querySelectorAll('.history-item').forEach(el => el.classList.remove('active-item'));
}

document.getElementById('modal-close').addEventListener('click', closeTrainingModal);

// Personal Records dynamic fetching and list interaction
async function loadRecords() {
    const listEl = document.getElementById('medals-list');
    if (!listEl) return;
    listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light);">Loading...</div>';

    try {
        const data = await apiCall('records', `/records/${currentUser.id}`);
        const records = data.records || [];

        if (!records || records.length === 0) {
            listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light); opacity: 0.6; font-style: italic;">No personal records yet</div>';
            return;
        }

        listEl.innerHTML = '';
        const curves = [
            "M 0,10 C 20,8 40,2 60,6 C 80,10 90,4 100,2",
            "M 0,14 C 20,12 40,6 60,8 C 80,4 90,2 100,1",
            "M 0,12 C 15,10 30,14 50,6 C 70,8 85,2 100,3"
        ];

        records.forEach((rec, index) => {
            let icon = '🏋️';
            const nameLower = (rec.exercise || '').toLowerCase();
            if (nameLower.includes('run') || nameLower.includes('sprint') || nameLower.includes('cardio') || nameLower.includes('treadmill')) {
                icon = '🏃';
            } else if (nameLower.includes('swim')) {
                icon = '🏊';
            } else if (nameLower.includes('cycle') || nameLower.includes('bike')) {
                icon = '🚴';
            }

            let medalClass = 'bronze';
            if (index === 0) medalClass = 'gold';
            else if (index === 1) medalClass = 'silver';

            const curve = curves[index % curves.length];

            const item = document.createElement('div');
            item.className = 'medal-item';

            item.innerHTML = `
                <div class="medal-circle ${medalClass}">${icon}</div>
                <div class="medal-info">
                    <div class="medal-header-row">
                        <span class="medal-title">${rec.exercise}: ${rec.best} kg</span>
                        <span class="medal-date">${rec.date || ''}</span>
                    </div>
                    <div class="sparkline-wrapper">
                        <svg class="sparkline" viewBox="0 0 100 16">
                            <path d="${curve}" fill="none" stroke="rgba(255,255,255,0.7)" stroke-width="2" stroke-linecap="round"></path>
                        </svg>
                    </div>
                </div>
            `;

            item.addEventListener('click', () => {
                // Hide training details when opening record details
                document.getElementById('training-modal').classList.add('hidden');
                showRecordModal(rec, item);
            });

            listEl.appendChild(item);
        });
    } catch (e) {
        listEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light); opacity: 0.6; font-style: italic;">No personal records yet</div>';
    }
}

async function showRecordModal(rec, activeItem) {
    document.querySelectorAll('.medal-item').forEach(el => el.classList.remove('active-item'));
    if (activeItem) activeItem.classList.add('active-item');

    document.getElementById('record-modal-title').textContent = 'Record History';
    document.getElementById('record-modal-exercise').textContent = rec.exercise;

    const histEl = document.getElementById('record-modal-history');
    histEl.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-light);">Loading history...</div>';

    document.getElementById('record-modal').classList.remove('hidden');

    try {
        const data = await apiCall('records', `/entries/${currentUser.id}/${encodeURIComponent(rec.exercise)}`);
        const history = data.history || [];

        let icon = '🏋️';
        const nameLower = (rec.exercise || '').toLowerCase();
        if (nameLower.includes('run') || nameLower.includes('sprint') || nameLower.includes('cardio') || nameLower.includes('treadmill')) {
            icon = '🏃';
        }

        if (history.length === 0) {
            histEl.innerHTML = '<div class="modal-empty">No records in history</div>';
        } else {
            // Sort history by date descending
            history.sort((a, b) => new Date(b.date) - new Date(a.date));

            histEl.innerHTML = history.map(h => {
                return `
                    <div class="modal-exercise-item">
                        <div class="modal-exercise-icon">${icon}</div>
                        <div>
                            <div class="modal-exercise-name">${h.weight} kg</div>
                            <div class="modal-exercise-details">Achieved on ${h.date}</div>
                        </div>
                    </div>
                `;
            }).join('');
        }
    } catch (e) {
        histEl.innerHTML = '<div class="modal-empty">Failed to load history</div>';
    }
}

function closeRecordModal() {
    document.getElementById('record-modal').classList.add('hidden');
    document.querySelectorAll('.medal-item').forEach(el => el.classList.remove('active-item'));
}

document.getElementById('record-modal-close').addEventListener('click', closeRecordModal);

// --- Create Personal Record Logic ---
document.getElementById('btn-add-record-trigger').addEventListener('click', async () => {
    // Set today's date as default
    const today = new Date().toISOString().split('T')[0];
    document.getElementById('record-date-input').value = today;
    document.getElementById('record-exercise-input').value = '';
    document.getElementById('record-weight-input').value = '';
    document.getElementById('create-record-overlay').classList.remove('hidden');

    // Fetch available exercises to populate the datalist autocomplete
    try {
        const data = await apiCall('records', `/available_exercises/${currentUser.id}`);
        const listEl = document.getElementById('available-exercises-list');
        listEl.innerHTML = '';
        
        // Add default exercises + whatever user has recorded
        const defaultExercises = ["Bench Press", "Squat", "Deadlift", "Overhead Press", "Barbell Row", "Pull-up", "Push-up", "Running", "Cycling"];
        let exercisesSet = new Set(defaultExercises);
        
        if (data.exercises && data.exercises.length > 0) {
            data.exercises.forEach(ex => exercisesSet.add(ex));
        }
        
        exercisesSet.forEach(ex => {
            const option = document.createElement('div');
            option.className = 'dropdown-option';
            option.textContent = ex;
            option.addEventListener('click', () => {
                document.getElementById('record-exercise-input').value = ex;
                listEl.classList.add('hidden');
            });
            listEl.appendChild(option);
        });
    } catch (e) {
        console.error("Failed to load available exercises", e);
    }
});

// Exercise Dropdown Interaction Logic
const recordExerciseInput = document.getElementById('record-exercise-input');
const recordExerciseList = document.getElementById('available-exercises-list');

recordExerciseInput.addEventListener('focus', () => {
    if (recordExerciseList.children.length > 0) {
        recordExerciseList.classList.remove('hidden');
    }
});

recordExerciseInput.addEventListener('input', () => {
    recordExerciseList.classList.remove('hidden');
    const filter = recordExerciseInput.value.toLowerCase();
    const options = recordExerciseList.querySelectorAll('.dropdown-option');
    options.forEach(opt => {
        if (opt.textContent.toLowerCase().includes(filter)) {
            opt.style.display = 'flex';
        } else {
            opt.style.display = 'none';
        }
    });
});

document.addEventListener('click', (e) => {
    if (e.target !== recordExerciseInput && e.target !== recordExerciseList && !recordExerciseList.contains(e.target)) {
        recordExerciseList.classList.add('hidden');
    }
});

document.getElementById('create-record-close').addEventListener('click', () => {
    document.getElementById('create-record-overlay').classList.add('hidden');
});

document.getElementById('btn-submit-record').addEventListener('click', async () => {
    const exercise = document.getElementById('record-exercise-input').value.trim();
    const weightStr = document.getElementById('record-weight-input').value;
    const date = document.getElementById('record-date-input').value;

    if (!exercise || !weightStr || !date) {
        showToast('Please fill all fields for the record', true);
        return;
    }

    const weight = parseFloat(weightStr);

    const body = {
        athlete_id: currentUser.id,
        exercise_name: exercise,
        weight: weight,
        date: date // Backend expects YYYY-MM-DD
    };

    try {
        await apiCall('records', '/entries', 'POST', body);
        showToast('Record saved successfully!');
        document.getElementById('create-record-overlay').classList.add('hidden');
        await loadRecords(); // Refresh the list
    } catch (e) {
        // apiCall already shows error toast
    }
});
// ------------------------------------

document.getElementById('btn-logout').addEventListener('click', () => {
    currentUser = null;
    document.getElementById('login-username').value = '';
    document.getElementById('login-password').value = '';
    document.getElementById('reg-fname').value = '';
    document.getElementById('reg-lname').value = '';
    document.getElementById('reg-dob').value = '';
    switchView('landing');
    if (!isLoginMode) toggleAuthMode(); // Reset to login mode
});

// Create Training State
let selectedExp = 3;
let selectedFocus = "Strength";
let selectedMuscle = "chest";

// Star Rating logic
document.querySelectorAll('#star-rating .star').forEach(star => {
    star.addEventListener('click', () => {
        selectedExp = parseInt(star.getAttribute('data-value'));
        updateStars();
    });
});

function updateStars() {
    document.querySelectorAll('#star-rating .star').forEach(s => {
        const val = parseInt(s.getAttribute('data-value'));
        if (val <= selectedExp) {
            s.classList.add('active');
        } else {
            s.classList.remove('active');
        }
    });
}

// Focus Pill Selector logic
document.querySelectorAll('.focus-pill').forEach(pill => {
    pill.addEventListener('click', () => {
        selectedFocus = pill.getAttribute('data-value');
        updateFocusSelector();
    });
});

function updateFocusSelector() {
    document.querySelectorAll('.focus-pill').forEach(p => {
        if (p.getAttribute('data-value') === selectedFocus) {
            p.classList.add('active');
        } else {
            p.classList.remove('active');
        }
    });

    // Show/hide muscle group select only for Strength
    const muscleContainer = document.getElementById('muscle-group-container');
    if (selectedFocus === "Strength") {
        muscleContainer.style.display = 'block';
    } else {
        muscleContainer.style.display = 'none';
    }
}

// Custom Muscle Dropdown logic
const dropdownTrigger = document.getElementById('dropdown-trigger');
const dropdownOptions = document.getElementById('dropdown-options');

dropdownTrigger.addEventListener('click', (e) => {
    e.stopPropagation();
    dropdownOptions.classList.toggle('hidden');
});

document.querySelectorAll('.dropdown-option').forEach(option => {
    option.addEventListener('click', (e) => {
        e.stopPropagation();
        const val = option.getAttribute('data-value');
        const text = option.querySelector('span').textContent;
        const icon = option.querySelector('.muscle-icon-circle').textContent;
        updateSelectedMuscle(val, text, icon);
        dropdownOptions.classList.add('hidden');
    });
});

function updateSelectedMuscle(val, text, icon) {
    selectedMuscle = val;
    document.getElementById('selected-muscle-text').textContent = text;
    document.getElementById('selected-muscle-icon').textContent = icon;
}

document.addEventListener('click', () => {
    dropdownOptions.classList.add('hidden');
});

// View Transitions
document.getElementById('btn-create-training').addEventListener('click', () => {
    document.getElementById('create-training-subview').classList.remove('hidden');

    // Set default date to today
    const today = new Date().toISOString().split('T')[0];
    document.getElementById('create-date').value = today;

    // Reset state to defaults
    selectedExp = 3;
    selectedFocus = "Strength";
    selectedMuscle = "chest";
    updateStars();
    updateFocusSelector();
    updateSelectedMuscle("chest", "Chest", "🏋️");
});

document.getElementById('btn-back-to-dashboard').addEventListener('click', (e) => {
    e.preventDefault();
    document.getElementById('create-training-subview').classList.add('hidden');
    loadTrainings();
});

// Confirm and POST Training logic
document.getElementById('btn-confirm-create').addEventListener('click', async () => {
    const dateVal = document.getElementById('create-date').value;
    if (!dateVal) {
        showToast('Please select a training date', true);
        return;
    }

    // Convert date format from YYYY-MM-DD to DD-MM-YYYY for SQLite mapping
    const parts = dateVal.split('-');
    const formattedDate = `${parts[2]}-${parts[1]}-${parts[0]}`;

    const body = {
        type: selectedFocus,
        date: formattedDate,
        exp: selectedExp,
        muscle: selectedFocus === "Strength" ? selectedMuscle : "-"
    };

    try {
        const btn = document.getElementById('btn-confirm-create');
        btn.disabled = true;
        btn.textContent = 'Creating...';

        await apiCall('training', `/athletes/${currentUser.id}/trainings`, 'POST', body);

        showToast('Workout successfully created!');

        // Return to dashboard immediately
        document.getElementById('create-training-subview').classList.add('hidden');

        // Reload list
        await loadTrainings();
    } catch (err) {
        // Handled in apiCall
    } finally {
        const btn = document.getElementById('btn-confirm-create');
        btn.disabled = false;
        btn.textContent = 'Confirm Workout';
    }
});

// Floating sidebar menu actions
document.getElementById('btn-sidebar-home').addEventListener('click', () => {
    document.querySelectorAll('.sidebar-icon-btn').forEach(btn => btn.classList.remove('active'));
    document.getElementById('btn-sidebar-home').classList.add('active');

    // Go to dashboard subview
    document.getElementById('create-training-subview').classList.add('hidden');

    // Scroll to the very top of the page smoothly
    window.scrollTo({ top: 0, behavior: 'smooth' });
});

document.getElementById('btn-sidebar-add').addEventListener('click', () => {
    document.querySelectorAll('.sidebar-icon-btn').forEach(btn => btn.classList.remove('active'));
    document.getElementById('btn-sidebar-add').classList.add('active');

    // Trigger creation click
    document.getElementById('btn-create-training').click();
});

document.getElementById('btn-sidebar-analytics').addEventListener('click', () => {
    document.querySelectorAll('.sidebar-icon-btn').forEach(btn => btn.classList.remove('active'));
    document.getElementById('btn-sidebar-analytics').classList.add('active');

    // Switch to dashboard subview if we are on the create-training view
    document.getElementById('create-training-subview').classList.add('hidden');

    // Smooth scroll to the Personal Records section, aligning it to the center of the viewport
    const recordsSection = document.getElementById('records-section-row');
    if (recordsSection) {
        recordsSection.scrollIntoView({ behavior: 'smooth', block: 'center' });
    }
});

// ============================================================
// SETTINGS PANEL
// ============================================================

function openSettings() {
    document.getElementById('settings-old-password').value = '';
    document.getElementById('settings-new-password').value = '';
    document.getElementById('settings-overlay').classList.remove('hidden');
}

function closeSettings() {
    document.getElementById('settings-overlay').classList.add('hidden');
}

document.getElementById('btn-sidebar-settings').addEventListener('click', () => {
    document.querySelectorAll('.sidebar-icon-btn').forEach(btn => btn.classList.remove('active'));
    document.getElementById('btn-sidebar-settings').classList.add('active');
    openSettings();
});

document.getElementById('settings-close').addEventListener('click', closeSettings);

// Close settings when clicking the backdrop
document.getElementById('settings-overlay').addEventListener('click', (e) => {
    if (e.target === document.getElementById('settings-overlay')) closeSettings();
});

// Change Password
document.getElementById('btn-change-password').addEventListener('click', async () => {
    const oldPass = document.getElementById('settings-old-password').value.trim();
    const newPass = document.getElementById('settings-new-password').value.trim();

    if (!oldPass || !newPass) {
        showToast('Please fill in both password fields', true);
        return;
    }

    if (newPass.length < 4) {
        showToast('New password must be at least 4 characters', true);
        return;
    }

    try {
        const btn = document.getElementById('btn-change-password');
        btn.disabled = true;
        btn.textContent = 'Updating...';

        await apiCall('auth', `/users/${currentUser.id}/password`, 'PUT', {
            old_password: oldPass,
            new_password: newPass
        });

        showToast('Password updated successfully! ✅');
        document.getElementById('settings-old-password').value = '';
        document.getElementById('settings-new-password').value = '';
        closeSettings();
    } catch (e) {
        // Error already shown in apiCall
    } finally {
        const btn = document.getElementById('btn-change-password');
        btn.disabled = false;
        btn.textContent = 'Update Password';
    }
});

// Logout from settings
document.getElementById('btn-settings-logout').addEventListener('click', () => {
    closeSettings();
    document.getElementById('btn-logout').click();
});

// Delete Account — removes user + all trainings + all records
document.getElementById('btn-delete-account').addEventListener('click', async () => {
    const confirmed = confirm(
        '⚠️ Are you sure you want to delete your account?\n\n' +
        'This will permanently delete:\n' +
        '  • Your account\n' +
        '  • All your trainings\n' +
        '  • All your personal records\n\n' +
        'This action cannot be undone!'
    );
    if (!confirmed) return;

    const btn = document.getElementById('btn-delete-account');
    btn.disabled = true;
    btn.textContent = 'Deleting...';

    try {
        const userId = currentUser.id;

        // 1. Delete all records from records_service
        try {
            await fetch(`${API_URLS.records}/athletes/${userId}/entries`, { method: 'DELETE' });
        } catch (e) { /* ignore if service is down */ }

        // 2. Delete all trainings from training_service
        try {
            await fetch(`${API_URLS.training}/athletes/${userId}/trainings`, { method: 'DELETE' });
        } catch (e) { /* ignore if service is down */ }

        // 3. Delete the user account from auth_service
        await apiCall('auth', `/users/${userId}`, 'DELETE');

        showToast('Account deleted. Goodbye! 👋');
        closeSettings();

        // Reset and return to landing
        currentUser = null;
        document.getElementById('login-username').value = '';
        document.getElementById('login-password').value = '';
        document.getElementById('reg-fname').value = '';
        document.getElementById('reg-lname').value = '';
        document.getElementById('reg-dob').value = '';
        switchView('landing');
        if (!isLoginMode) toggleAuthMode();

    } catch (e) {
        // Error shown in apiCall
        btn.disabled = false;
        btn.textContent = '🗑️ Delete My Account';
    }
});
