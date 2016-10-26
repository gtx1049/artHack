package com.example.tomek.notepad;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.ContextMenu;
import android.view.MenuInflater;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.Toast;

import java.util.ArrayList;

/**
 * Main Activity class
 */
public class MainActivity extends AppCompatActivity {

    // Array used to backup data before using search function
    ArrayList<Note> allNotesSearchArray;

    // Database Handler
    private DatabaseHandler dbHandler;

    // Alert dialogs for back button and delete all notes button
    private AlertDialog alertDialogCloseApp;
    private AlertDialog alertDialogDeleteAll;
    private AlertDialog alertDialogDeleteSingleNote;

    // Note selected on menu
    private Note selectedNote;

    // Variables used to handle note list
    public static NoteAdapter noteAdapter; // TODO is static ok?
    public static ListView listView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Create DatabaseHandler
        dbHandler = new DatabaseHandler(getApplicationContext());

        // Add items to ListView
        listView = (ListView) findViewById(R.id.listView);
        populateListView(dbHandler.getAllNotesAsArrayList());

        // Assign listView to context menu
        registerForContextMenu(listView);

        // Setup AlertDialogs
        alertDialogDeleteAll = initAlertDialogDeleteAllNotes();
        alertDialogCloseApp = initAlertDialogCloseApp();

        // Floating Action Button listener used to adding new notes
        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                hideSoftKeyboard();
                Intent intent = new Intent(MainActivity.this, NoteActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                intent.putExtra("id", "-1");
                startActivity(intent);
            }
        });

        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                selectedNote = ((Note) parent.getAdapter().getItem(position));
                editNote(selectedNote.getId());
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Creating menu
        getMenuInflater().inflate(R.menu.menu_main, menu);

        SearchView searchView = (SearchView) menu.findItem(R.id.search).getActionView();
        searchView.setQueryHint("Search notes...");

        searchView.setOnSearchClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                allNotesSearchArray = noteAdapter.getData();
            }
        });

        final SearchView.OnQueryTextListener queryTextListener = new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextChange(String newText) {

                ArrayList<Note> filteredNotesArrayList = new ArrayList<>();
                for (Note note : allNotesSearchArray) {
                    if (note.getRawText().contains(newText)) {
                        filteredNotesArrayList.add(note);
                    }
                }

                populateListView(filteredNotesArrayList);
                noteAdapter.notifyDataSetChanged();

                return true;
            }

            @Override
            public boolean onQueryTextSubmit(String query) {
                // Do nothing
                return true;
            }
        };
        searchView.setOnQueryTextListener(queryTextListener);

        return true;
    }

    /**
     * Method used for first setup of back button AlertDialog
     */
    private AlertDialog initAlertDialogCloseApp() {//关闭app的函数

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Are you sure?").setTitle("Close Application");
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (android.os.Build.VERSION.SDK_INT >= 16) {
                    MainActivity.this.finishAffinity();
                }
            }
        });

        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        return builder.create();
    }
    /**
     * Method used for first setup of delete all notes button AlertDialog
     */
    private AlertDialog initAlertDialogDeleteAllNotes() {//删除全部note功能

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Are you sure?").setTitle("Delete all notes");
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                deleteAllNotes();
                Toast.makeText(MainActivity.this, "All notes has been deleted!",
                        Toast.LENGTH_SHORT).show();
            }
        });

        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        return builder.create();
    }

    /**
     * Method used for setup of delete single note button AlertDialog
     */
    private AlertDialog setupAlertDialogDeleteSingleNote() {

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Are you sure?").setTitle("Delete note #" + selectedNote.getId());
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dbHandler.deleteNote(selectedNote);
                noteAdapter.remove(selectedNote);
                noteAdapter.notifyDataSetChanged();
                Toast.makeText(MainActivity.this, "Note #" + selectedNote.getId() + " deleted",
                        Toast.LENGTH_SHORT).show();
            }
        });

        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        return builder.create();
    }

    /**
     * Method used to show AlertDialog when delete all notes button is clicked
     */
    public void showAlertDialogDeleteAllNotes(MenuItem menuItem) {
        alertDialogDeleteAll.show();
    }

    /**
     * Method used to show AlertDialog when delete note button is clicked
     */
    private void showAlertDialogDeleteSingleNote() {
        alertDialogDeleteSingleNote.show();
    }

    /**
     * Method that Overrides back button behavior
     * When back button is pressed it shows "back button" AlertDialog
     */
    @Override
    public void onBackPressed() {
        alertDialogCloseApp.show();
    }

    /**
     * Method used to delete all notes via DatabaseHandler
     */
    public void deleteAllNotes() {
        dbHandler.clearAllNotes();
        noteAdapter.clear();
        noteAdapter.notifyDataSetChanged();
    }

    /**
     * Method used to enter note edition mode
     *
     * @param noteId
     */
    private void editNote(int noteId) {
        hideSoftKeyboard();
        Intent intent = new Intent(MainActivity.this, NoteActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        intent.putExtra("id", String.valueOf(noteId));
        startActivity(intent);
    }

    /**
     * Method used to hide keyboard
     */
    private void hideSoftKeyboard() {
        if (this.getCurrentFocus() != null) {
            try {
                InputMethodManager imm = (InputMethodManager) this.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(this.getCurrentFocus().getApplicationWindowToken(), 0);
            } catch (RuntimeException e) {
                //ignore
            }
        }
    }

    /**
     * Method used to fill ListView
     *
     * @param note Array of Notes containing all Notes in Database
     */
    private void populateListView(ArrayList<Note> note) {
        noteAdapter = new NoteAdapter(this,
                R.layout.listview_item_row, note);
        listView.setAdapter(noteAdapter);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
                                    ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);

        if (v.getId() == R.id.listView) {
            ListView listView = (ListView) v;
            AdapterView.AdapterContextMenuInfo acmi = (AdapterView.AdapterContextMenuInfo) menuInfo;
            selectedNote = (Note) listView.getItemAtPosition(acmi.position);
            menu.setHeaderTitle("Choose action for note #" + selectedNote.getId());
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.context_menu_note_select, menu);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case R.id.context_menu_delete:
                alertDialogDeleteSingleNote = setupAlertDialogDeleteSingleNote();
                showAlertDialogDeleteSingleNote();
                break;
            case R.id.context_menu_edit:
                editNote(selectedNote.getId());
                break;
        }
        return super.onContextItemSelected(item);
    }
}
